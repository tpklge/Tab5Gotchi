# Continuidade

O núcleo está implementado, compilado, instalado e testado no Tab5. Entrega em dist e evidências em hardware.md. Tab5Gotchi já ocupa app1: não repetir instalação sem conferir inventário. Para atualizar, usar somente o aplicativo pelo launcher ou tools/flash_app.py após ler o mapa real. Nunca gravar a tabela/bootloader do build por cima do launcher.

1. Validar manualmente toque e som; medir autonomia e RTC sem alimentação, e ensaiar cortes de energia com progresso isolado.
2. Medir e otimizar apresentação até 30 fps sem consultas USB/capturas; a regressão instrumentada atingiu aproximadamente 25 fps no jogo de frutas.
3. Com dois aparelhos e slot adequado, validar a variante extras em rede local e o fluxo completo de visitas/presentes. Nenhuma atualização do firmware do C6 é feita pelo jogo.
4. Para conteúdo novo, editar assets_src/generate.py, gerar, validar e copiar seletivamente, preservando saves/álbum e IDs existentes. Mudança do layout binário de State exige versão e migração explícita.

Backup integral e logs pessoais ficam em private/hardware, excluídos dos pacotes. Os documentos originais de concepção permanecem em Docs.
