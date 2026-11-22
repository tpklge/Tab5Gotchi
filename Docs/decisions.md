# Decisões

- ESP-IDF 5.4.4, BSP oficial espressif/m5stack_tab5 1.3.2, esp_lvgl_port 2.9.0, LVGL 9.3.0: versões já compiladas no launcher. Sem Arduino nem rede no jogo básico.
- Projeto isolado. Não copiar código dos aplicativos vizinhos. BSP Apache-2.0 mantido pelo Component Manager.
- FatFs exFAT ativado com cabeçalho de configuração por build, com ABI única para todos os componentes; o cartão conectado foi identificado como FAT32.
- Mapa local serve somente ao build. Instalação será app-only pelo launcher ou slot livre previamente conferido. Nunca gravar bootloader/tabela gerados.
- Pixel art desenhada offline por scripts com desenhos próprios por família/forma, animação por poses e efeitos. WAV PCM mono 22050 Hz sintetizado offline.
- Simulação determinística independente do hardware; três fluxos de aleatoriedade separados. Persistência A/B com CRC e validação após flush. FAT não oferece garantia absoluta contra perda de energia.

- Teste físico encontrou estouro de pilha na inicialização do áudio: buffers de mixagem movidos para armazenamento estático e pilha aumentada para 12 KiB. Também corrigida ordem do cabeçalho FatFs: sdkconfig deve preceder ffconf para habilitar nomes longos.
- Segunda regressão física detectou cópias de State grandes demais na pilha ao salvar. Os buffers temporários A/B, validação e rollback agora usam heap/PSRAM com vida limitada. O aviso de nomes longos do BSP 1.3.2 consulta um símbolo antigo e pode aparecer mesmo com CONFIG_FATFS_LFN_HEAP ativo; conferir o ffconf efetivo em vez de inferir pelo aviso.
- Checkpoint físico de 120 segundos revelou que rename do FatFs não substitui destino existente. Após validar o temporário, agora removemos somente o slot mais antigo antes do rename, mantendo o outro válido. O erro anterior pausou o jogo e preservou o último checkpoint.
- LVGL passa a usar malloc do sistema/PSRAM. O heap interno padrão de 64 KiB não comportava capturas e tinha margem pequena para a interface completa. Captura e retrato são verificados na revisão seguinte.
