# Stream Monitor Relay (webshot)

Servidor **relay WebSocket** do Stream Monitor do Acrisio — a peça do meio que liga
os **capturadores** (clientes PangYa que mandam a tela) aos **viewers**
(`www/pangya/gm_tools/stream_monitor.php`, a página do GM).

```
  [Cliente PangYa]                [Relay Node]              [GM no navegador]
   capturador DLL   --frames-->   server.js  --frames-->   stream_monitor.php
   (Fase 2)         <-comandos--  (porta 4573) <-comandos-- (viewer)
```

## Rodar

```
npm install          # instala o ws
npm start            # sobe o relay em ws://0.0.0.0:4573
```

Variáveis de ambiente: `WEBSHOT_PORT` (padrão 4573), `WEBSHOT_HOST` (padrão 0.0.0.0).

## Testar sem a DLL (capturador simulado)

Em 3 terminais:
```
npm start                              # 1) relay
node test_capturer.js 4484 ADM_SuperSS # 2) capturador falso (manda JPEG 1x1)
node test_viewer.js 4484               # 3) viewer CLI (loga frames + testa FOCUS)
```
Ou abra `test_viewer.html` (viewer web local, mesma lógica do .php do Acrisio,
apontando pra `127.0.0.1:4573`) num navegador com o relay + capturador rodando.

✅ **Validado**: capturador → relay → viewer (frames) e viewer → relay → capturador
(comandos FOCUS/RATE), inclusive renderizando no viewer web do Acrisio.

## Protocolo

Ver o cabeçalho de `server.js` — está tudo documentado (mensagens de viewer,
de capturador e o que o relay repassa em cada direção).

## Próximos passos

- **Fase 2**: DLL capturadora no cliente (GDI BitBlt → JPEG → WebSocket).
- **Fase 3**: apontar o `stream_monitor.php` pra este relay (hoje ele tem
  `HOST_WEBSHOT = 'superss.ga'` / porta 4573 hardcoded).
