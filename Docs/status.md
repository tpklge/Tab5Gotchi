# Estado de implementação — 22/11/2026

## Implementado

Motor C++ compartilhado, seis famílias/48 formas/seis ovos, variações e animações, cuidados, oito jogos, três perfis, modos de tempo, perfil/álbum/diário/histórias/decoração/retratos, tema retrô, A/B/C, configurações, sons e streaming de música, persistência A/B, recuperação, geradores, validador e instalador de recursos.

Recursos reais: pacote com 179 arquivos imutáveis, 54 bancos de formas/ovos, dez cenários, 48 WAVs, duas fontes, ícones de cuidados/alimentos/decoração, sprites dos jogos e assinaturas herdadas nos idosos. O validador rejeita formas idênticas dentro de uma família. Conteúdo não está embutido no firmware.

Extensões: herança local e visitas/presentes UDP isoladas, desativadas no firmware padrão. Variante extras compila para ESP32-P4; protocolo de visitas testado entre dois processos do host. Integração entre dois Tab5 ainda não validada.

## Validado no host

ASan/UBSan: persistência, identidade, cuidados, banho/saúde/sono, limites, anti-spam, evolução e seus 36 caminhos, apresentações pendentes, tempo e jogos. Testes separados de herança e protocolo UDP. Simulação de 15 dias usa o mesmo motor. Validação de hashes, referências, imagens/animações, cobertura e áudio do pacote passou.

## Compilado para o alvo

ESP-IDF 5.4.4, BSP 1.3.2, LVGL 9.3.0, port 2.9.0. Firmware padrão e variante extras compilados; relatório por componente em build-size.txt. Binário padrão de 911296 bytes; variante extras de 1250016 bytes. Firmware é somente aplicativo.

## Testes reais no Tab5

- Tab5 revisão 2, ST7123/touch FW3, ESP32-P4 revisão 1.3, 32 MiB PSRAM, codec ES8388 e BMI270 identificados.
- Backup integral inicial verificado pelo launcher e preservado em private/hardware.
- Instalação em app1 por fluxo do launcher. Meshtastic em app0 preservado. Uma duplicata temporária do jogo foi removida pelo próprio launcher.
- Falhas encontradas e corrigidas: pilha do áudio, temporários grandes de State na pilha, substituição de arquivo no FAT e heap de captura LVGL.
- Ovo recuperado com a mesma identidade em múltiplos reinícios; eclosão normal de Brasquito observada. Checkpoints A/B passaram de duas gravações e a simulação seguiu sem falha.
- Capturas BMP/PNG reais do framebuffer obtidas pela USB. Blocos de áudio enviados ao codec sem erros no período observado. Sensor BMI270 inicializado.
- Recursos finais atualizados pela WebGUI com verificação e preservação de saves/álbum.
- Regressão de cuidados, oito jogos, três perfis, restauração, retratos, álbum, retrô e A/B/C aprovada no aparelho. Sequência musical chegou a dez níveis e Memória a oito pares. Cinco reinícios de Pega-frutinha e recompensa única conferidos após o último ajuste de UI. Entrada foi injetada por USB no controlador/LVGL; não comprova gesto humano no sensor.

Medição instrumentada em Pega-frutinha: aproximadamente 23–25 atualizações visuais/s; meta de 30 fps ainda não atingida consistentemente. Zero erros de áudio reportados. Evidências e medições em [hardware.md](hardware.md).

## Limites ainda não comprovados

Audição humana, autonomia, retenção do RTC sem alimentação, teste entre dois Tab5 e ensaio prolongado de cortes de energia. Não foram alegados como aprovados. Logs privados e backups pessoais nunca entram nos pacotes de distribuição.

## Revisão da interface

Fundos e controles em cinza neutro, alinhamentos e margens uniformes, feedback de pressão e menus Saúde/Educação separados. Build padrão/extras aprovados; abertura e navegação conferidas no Tab5, preservando identidade e 16 partidas. Captura em screenshots/interface-cinza.png.

## Revisão touch e perfis

Controles A/B/C retirados. Ficha sem sobreposição; submenus em lista com botões de largura inteira; Ajustes organizado por categorias. Entrada de nomes pelo teclado original A164, com Enter/Esc/Backspace e setas; sem teclado virtual na tela de nomes. Detecção do acessório ainda não confirmada nesta sessão: não se alega digitação física validada.

Confirmados no Tab5: navegação, cancelamento da exclusão, exclusão confirmada, espaço vazio persistente após reiniciar e criação de novo ovo. Os dois saves originais do perfil de teste foram copiados antes do ensaio e restaurados com comparação de bytes. Exclusão normal arquiva o ciclo em album/deleted-*.sav antes de liberar o espaço.

## Layout inspirado em apps Android

Referências e decisões em android-layout.md. Casa reorganizada com seis ações na base, quatro indicadores interativos à direita, aviso próprio no topo da cena e navegação com seção ativa. Teste no Tab5 conferiu cinco botões de navegação, três indicadores, ficha, educação e perfis por injeção de toque, mantendo a identidade do pet. Captura real em screenshots/android-home.png. Padrão e extras compilados.
