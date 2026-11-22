# Estado de implementação — 22/11/2026

## Implementado

Motor C++ compartilhado, seis famílias/48 formas/seis ovos, variações e animações, cuidados, oito jogos, três perfis, modos de tempo, perfil/álbum/diário/histórias/decoração/retratos, tema retrô, controles touch, configurações, sons e streaming de música, persistência A/B, recuperação, geradores, validador e instalador de recursos.

Recursos reais: pacote com 185 arquivos imutáveis, 54 bancos de formas/ovos, dez cenários, 48 WAVs, duas fontes, ícones de cuidados/alimentos/decoração, sprites dos jogos e assinaturas herdadas nos idosos. O validador rejeita formas idênticas dentro de uma família. Conteúdo não está embutido no firmware.

Extensões: herança local e visitas/presentes UDP isoladas, desativadas no firmware padrão. Variante extras compila para ESP32-P4; protocolo de visitas testado entre dois processos do host. Integração entre dois Tab5 ainda não validada.

## Validado no host

ASan/UBSan: persistência, identidade, cuidados, banho/saúde/sono, limites, anti-spam, evolução e seus 36 caminhos, apresentações pendentes, tempo e jogos. Testes separados de herança e protocolo UDP. Simulação de 15 dias usa o mesmo motor. Validação de hashes, referências, imagens/animações, cobertura e áudio do pacote passou.

## Compilado para o alvo — revisões anteriores

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

## Gestão de bateria e tela

Cabeçalho com porcentagem aproximada para a bateria NP-F550 2S, aviso em 15% ou menos e leitura de tensão em Tela e conforto. Sem leitura válida, mostra indisponível. Volume e brilho usam botões de `-`/`+` com limites; apagamento por inatividade configurável entre 30 s e 30 min, opção Nunca e padrão de 2 min. Reativação por toque/teclado consome a primeira entrada, preservando brilho e progresso. Minijogos ativos mantêm a tela ligada. Tela apagada mantém simulação e checkpoints, suspendendo atualizações visuais.

Testes host com ASan/UBSan aprovados para bateria/inatividade, migração dos ajustes v1 para v2 e recuperação A/B. Compilação padrão ESP32-P4 aprovada nesta revisão. Na primeira compilação, os testes físicos estavam pendentes; os resultados de apagamento e reativação obtidos depois estão na revisão 0.2.0 abaixo. Precisão da estimativa e consumo continuam pendentes.

## Revisão 0.2.0 — fechamento das pendências de software

Implementados: sequências de animação e geometria por metadados; silhuetas; histórico de indivíduos com aparência, marcos e parentesco; descobertas por RNG de eventos; limites de evolução por atividade/etapa, variedade e qualidade; retorno decrescente de ações sociais; balanceamento/checkpoint em JSON; dia/noite contínuo, transições e efeito de transformação; esconde-esconde com arte e revelação; hashes de recursos e diagnóstico de áudio.

State v2 migra v1 com CRC/validação e preservação da identidade; datas indisponíveis não são inventadas. Host ASan/UBSan, simulação de 15 dias e geração reproduzível/validação dos 185 arquivos aprovados. Builds padrão e extras aprovados. A instalação foi feita em etapas pelo launcher no slot 2, com backups, verificação dos bytes e dos outros aplicativos; o firmware do launcher não foi substituído.

No aparelho: identidade preservada na migração, navegação/álbum/histórico, silhuetas e esconde-esconde capturados; apagamento em 30 s, despertar sem ação subjacente, checkpoint/simulação durante 65 s apagado e Nunca durante 33 s aprovados por injeção LVGL. Após corrigir invalidação indevida da camada de iluminação, Pega-frutinha sustentou aproximadamente 33 fps em janelas de dez segundos, sem capturas durante a medição e sem erros do codec. Audição humana e autonomia não são inferidas desses contadores.

Validação manual informada pelo usuário nesta sessão: teclado, touch e reinícios aleatórios pelo botão funcionando. Permanecem ensaios físicos específicos: autonomia/precisão da estimativa de bateria, RTC sem alimentação, cortes controlados em fases de escrita e visitas entre dois Tab5. Não há segundo Tab5 identificado para validar a extensão física.

Ensaio final adicional aprovado: pacote incompatível bloqueou a abertura sem alterar qualquer arquivo de save (comparação por SHA-256); recursos restaurados e pet recuperado. Capturas finais em screenshots/0.2-home.png, 0.2-album.png e 0.2-history.png.

## Correção 0.2.1 — teclado ao mudar nome

Detecta o A164 pela leitura de seu registrador de identificação, com timeout de 50 ms, sem sondagem de endereço isolada. Consome lotes da fila HID e reconhece maiúsculas, pontuação, Enter/Esc, Backspace/Delete, setas, Home/End e Ctrl+A para limpar o rascunho. Texto e posição do cursor são preservados durante reconstruções da tela. A tela informa a conexão do teclado.

Testes host de HID e regressões anteriores aprovados. No Tab5: teclado físico detectado e testes do mesmo handler por USB aprovados para edição, símbolos, cursor, reconstrução, Enter e Esc, sem alterar o nome salvo. O usuário confirmou nesta sessão que a digitação física na tela Mudar nome está funcionando corretamente. Instalada pelo launcher no slot 2 com backup.
