/*
 * Stream Monitor Relay (webshot)
 * --------------------------------
 * Hub WebSocket que liga os CAPTURADORES (clientes PangYa que mandam a tela)
 * aos VIEWERS (a pagina gm_tools/stream_monitor.php do GM).
 *
 * Protocolo (deduzido do stream_monitor.php do Acrisio):
 *
 *   VIEWER -> RELAY (comandos; o .php ja manda exatamente estes):
 *     { type:"FOCUS-4",  id }           foca o player (qualidade/rate normal)
 *     { type:"FOCUS-10", id }           foca (ctrl+click, rate maior)
 *     { type:"MINI-FRAME", id }         volta o player pra miniatura
 *     { type:"TOGGLE-SEND-FRAMES", id } liga/desliga o envio de frames
 *     { type:"TOGGLE-CAPTURE", id }     alterna captura DirectX <-> GDI+
 *     { type:"CLOSE", id }              fecha o stream daquele player
 *   (o viewer NAO manda nada ao conectar: ele so RECEBE frames)
 *
 *   RELAY -> VIEWER (frames; o .php espera exatamente isto):
 *     { id, nickname, frame:<jpeg base64>, type:0 }   frame novo
 *     { id, type:3, frame:null, close:true }          player saiu (remove do mosaico)
 *
 *   CAPTURADOR -> RELAY (protocolo NOSSO; a DLL implementa):
 *     { type:"REGISTER", id, nickname }               anuncia quem e
 *     { type:"FRAME", id, nickname, frame:<base64> }  manda um frame
 *
 *   RELAY -> CAPTURADOR (repassa a intencao do GM):
 *     { type:"RATE", value:4|10|"mini" }   qualidade/frequencia desejada
 *     { type:"TOGGLE-SEND" }               pausa/retoma envio
 *     { type:"TOGGLE-CAPTURE" }            troca metodo de captura
 *     { type:"CLOSE" }                     encerra
 *
 * Regra de papel: toda conexao comeca como VIEWER (assim um viewer silencioso
 * ja recebe frames). Vira CAPTURADOR assim que manda REGISTER/FRAME.
 * O 'id' amarra tudo: e o identificador do stream de um player (ex.: UID).
 */

'use strict';

const WebSocket = require('ws');
const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = parseInt(process.env.WEBSHOT_PORT || '4573', 10);
const HOST = process.env.WEBSHOT_HOST || '0.0.0.0';

/** @type {Map<string, {ws: WebSocket, nickname: string}>} */
const capturers = new Map();   // id -> capturador
/** @type {Set<WebSocket>} */
const viewers = new Set();     // viewers (GM). Toda conexao entra aqui por padrao.

const now = () => new Date().toISOString().substr(11, 12);
const log = (...a) => console.log(`[${now()}]`, ...a);

// Servidor HTTP: serve o viewer de teste em GET / (facilita testar num navegador).
// O WebSocket sobe no MESMO servidor/porta (upgrade), entao a pagina e o ws
// compartilham host:porta. O stream_monitor.php de producao usa a mesma porta.
const httpServer = http.createServer((req, res) => {
    if (req.method === 'GET' && (req.url === '/' || req.url === '/index.html' || req.url === '/test_viewer.html')) {
        try {
            const html = fs.readFileSync(path.join(__dirname, 'test_viewer.html'));
            res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
            res.end(html);
        } catch (e) {
            res.writeHead(500); res.end('viewer nao encontrado');
        }
        return;
    }
    res.writeHead(404); res.end('nada aqui (use / pra o viewer de teste)');
});

const wss = new WebSocket.Server({ server: httpServer });

httpServer.listen(PORT, HOST, () => {
    log(`Stream Monitor Relay ouvindo em ws://${HOST}:${PORT}  (WebSocket + HTTP)`);
    log(`Viewer de teste: http://127.0.0.1:${PORT}/`);
    log(`Producao: aponte o stream_monitor.php pra este host/porta.`);
});

function safeSend(ws, obj) {
    if (ws && ws.readyState === WebSocket.OPEN) {
        try { ws.send(JSON.stringify(obj)); } catch (e) { /* socket morto */ }
    }
}

function broadcastToViewers(obj) {
    const msg = JSON.stringify(obj);
    for (const v of viewers) {
        if (v.readyState === WebSocket.OPEN) {
            try { v.send(msg); } catch (e) { /* ignora */ }
        }
    }
}

function promoteToCapturer(ws, id, nickname) {
    viewers.delete(ws);            // deixa de ser viewer
    ws._role = 'capturer';
    ws._id = id;
    const existing = capturers.get(id);
    if (existing && existing.ws !== ws) {
        // outra conexao ja usa esse id: derruba a antiga
        try { existing.ws.close(); } catch (e) { /* ignora */ }
    }
    capturers.set(id, { ws, nickname: nickname || (existing ? existing.nickname : '') });
}

// traduz o comando do viewer no comando pro capturador
function routeViewerCommand(msg) {
    const target = capturers.get(String(msg.id));
    if (!target) return;
    switch (msg.type) {
        case 'FOCUS-4':  safeSend(target.ws, { type: 'RATE', value: 4 });  break;
        case 'FOCUS-10': safeSend(target.ws, { type: 'RATE', value: 10 }); break;
        case 'MINI-FRAME': safeSend(target.ws, { type: 'RATE', value: 'mini' }); break;
        case 'TOGGLE-SEND-FRAMES': safeSend(target.ws, { type: 'TOGGLE-SEND' }); break;
        case 'TOGGLE-CAPTURE': safeSend(target.ws, { type: 'TOGGLE-CAPTURE' }); break;
        case 'CLOSE': safeSend(target.ws, { type: 'CLOSE' }); break;
        default: break;
    }
}

wss.on('connection', (ws, req) => {

    const peer = (req && req.socket && req.socket.remoteAddress) || '?';
    ws._role = 'viewer';   // padrao: viewer (recebe frames de imediato)
    ws._id = null;
    viewers.add(ws);
    log(`+ conexao de ${peer} (viewer por padrao; vira capturador se registrar)`);

    ws.on('message', (data) => {
        let msg;
        try { msg = JSON.parse(data); } catch (e) { return; }
        if (!msg || typeof msg !== 'object') return;

        switch (msg.type) {

            // ---- CAPTURADOR ----
            case 'REGISTER': {
                const id = String(msg.id);
                promoteToCapturer(ws, id, msg.nickname);
                log(`  capturador REGISTER id=${id} nick="${msg.nickname || ''}"`);
                break;
            }
            case 'FRAME': {
                const id = String(msg.id);
                if (ws._role !== 'capturer') promoteToCapturer(ws, id, msg.nickname);
                else if (msg.nickname && capturers.has(id)) capturers.get(id).nickname = msg.nickname;
                broadcastToViewers({
                    id,
                    nickname: msg.nickname || (capturers.get(id) || {}).nickname || '',
                    frame: msg.frame,
                    type: 0
                });
                break;
            }

            // ---- VIEWER (comandos do .php) ----
            case 'FOCUS-4':
            case 'FOCUS-10':
            case 'MINI-FRAME':
            case 'TOGGLE-SEND-FRAMES':
            case 'TOGGLE-CAPTURE':
            case 'CLOSE':
                routeViewerCommand(msg);
                break;

            default:
                break;
        }
    });

    ws.on('close', () => {
        if (ws._role === 'capturer' && ws._id != null) {
            const cur = capturers.get(ws._id);
            if (cur && cur.ws === ws) {
                capturers.delete(ws._id);
                broadcastToViewers({ id: ws._id, type: 3, frame: null, close: true });
                log(`- capturador saiu id=${ws._id}`);
            }
        } else {
            viewers.delete(ws);
            log(`- viewer saiu (restam ${viewers.size})`);
        }
    });

    ws.on('error', () => { /* silencioso: o close limpa */ });
});

setInterval(() => {
    log(`status: ${capturers.size} capturador(es), ${viewers.size} viewer(s)`);
}, 30000).unref();

process.on('SIGINT', () => { log('encerrando...'); wss.close(() => process.exit(0)); });
