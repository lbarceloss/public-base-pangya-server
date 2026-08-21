/*
 * Viewer SIMULADO (CLI) - imita o que o stream_monitor.php faz.
 * Conecta, loga os frames recebidos e (opcional) manda um comando de foco
 * depois de 2s pra testar o roteamento viewer -> capturador.
 *
 * uso:  node test_viewer.js [idParaFocar]
 */
'use strict';
const WebSocket = require('ws');

const URL = process.env.WEBSHOT_URL || 'ws://127.0.0.1:4573';
const focusId = process.argv[2] || null;

const ws = new WebSocket(URL);
let frames = 0;

ws.on('open', () => {
    console.log(`[viewer] conectado em ${URL} (silencioso, so recebendo)`);
    if (focusId) {
        setTimeout(() => {
            console.log(`[viewer] mandando FOCUS-4 no id=${focusId}`);
            ws.send(JSON.stringify({ type: 'FOCUS-4', id: focusId }));
        }, 2000);
    }
});

ws.on('message', (data) => {
    const evt = JSON.parse(data);
    if (evt.close) {
        console.log(`[viewer] player id=${evt.id} SAIU (close)`);
        return;
    }
    frames++;
    const len = evt.frame ? evt.frame.length : 0;
    if (frames <= 3 || frames % 10 === 0)
        console.log(`[viewer] frame #${frames} id=${evt.id} nick="${evt.nickname}" jpegBase64Len=${len} type=${evt.type}`);
});

ws.on('error', (e) => console.error('[viewer] erro:', e.message));
