Estado atual: revisão 0.2.1 no app2; as primeiras seções abaixo preservam ensaios anteriores. Veja a seção final para os resultados desta revisão.

# Validação no Tab5 — 22/11/2026

Firmware padrão instalado e funcionando com o launcher do Paludo. Fonte e dados são do projeto Tab5Gotchi; os documentos originais de concepção foram preservados. O aparelho foi deixado na casa do perfil 1, modo tranquilo, com a mesma identidade e progresso recuperados.

## Compilação e instalação

ESP-IDF 5.4.4, alvo ESP32-P4, BSP Tab5 1.3.2, LVGL 9.3.0 e port 2.9.0. Compilação padrão: **911296 bytes**, somente aplicativo. SHA-256: `e2afa19779901e9a8c5036eb7523686bc5b606a86ed83c4c0ba5052d069c4049`. A variante extras compilou com **1250016 bytes**; não foi instalada nem validada fisicamente. Seu tamanho exige slot maior pelo gerenciador do launcher.

O launcher instalou o jogo no app1. As atualizações posteriores leram a tabela real, verificaram descritores e gravaram exclusivamente esse aplicativo. Reserva app1: 0x3a0000, 917504 bytes; folga final: 6208 bytes. Esses endereços descrevem este aparelho, não um comando genérico de instalação. Não foi gravada a tabela/bootloader do build. O mapa de link por biblioteca está em [build-size.txt](build-size.txt) e [build-size.json](build-size.json).

Backup integral inicial mantido em privado. Após as gravações finais, leituras reais de flash foram comparadas byte a byte com esse backup:

| Região | Endereço inicial | Bytes lidos | Resultado |
| --- | --- | ---: | --- |
| bootloader | 0x2000 | 24576 | Igual ao backup |
| launcher | 0x20000 | 2031616 | Igual ao backup |
| meshtastic | 0x210000 | 1638400 | Igual ao backup |

Meshtastic permanece no app0. Foram preservados outros aplicativos, saves, álbum e arquivos pessoais do cartão. Nenhum backup pessoal ou progresso entra na distribuição.

## Aparelho e periféricos

Tab5 revisão 2; ESP32-P4 revisão 1.3; 32 MiB PSRAM; LCD ST7123 e touch ST7123 firmware 3 identificados pelo BSP. microSD FAT32 montado. Codec ES8388 e IMU BMI270 inicializados. RTC foi lido sem alterar sua configuração; tensão observada pelo INA226 ficou próxima de 8,3 V. Isso não mede autonomia nem comprova retenção do RTC sem alimentação.

## Comportamentos verificados

- Ovo com identidade persistente em reinícios e eclosão normal após sua incubação; não se acelerou o firmware.
- Alimentação, água, sono, ninar, abertura/interrupção de soneca, banho em etapas e recusa de remédio sem indicação.
- Três perfis, retorno à identidade anterior, exportação e restauração confirmada. A/B continuou gravando além de dois checkpoints no FAT real.
- Oito minijogos concluídos no aparelho: Jokenpô, Par/ímpar, Esquerda/direita, Maior/menor, Memória com oito pares, Sequência musical até dez níveis, Pega-frutinha e Esconde-esconde.
- Cada conclusão conferiu contador antes/depois e recusou nova recompensa para a mesma partida encerrada. Na versão final, cinco reinícios de Pega-frutinha precederam uma nova conclusão, sem duplicação da recompensa.
- Álbum, ficha, retrato BMP, retro e navegação A/B/C. Capturas reais obtidas do framebuffer pela USB; a faixa de avisos do A/B/C foi conferida após o ajuste final.
- Retorno ao launcher e nova abertura preservaram identidade, perfil e contador de partidas. O jogo ficou em execução na casa.
- Atualização seletiva de recursos pela WebGUI com readback SHA-256, manifesto por último e preservação de saves/álbum.

A entrada automatizada usa cliques no controlador do jogo e eventos de ponteiro LVGL. Não é um ensaio manual de dedo no sensor. A regressão foi concluída em etapas; erros do roteiro de observação das notas e um toque involuntário no botão de replay foram corrigidos no teste antes da repetição. Os oito resultados e a revisão final constam em [device-tests.txt](device-tests.txt).

## Medições

Payload de save: 4872 bytes. No conjunto instrumentado de cenas, perfis e jogos, heap interna livre ficou entre 460383 e 473107 bytes; PSRAM livre entre 29137512 e 29424800 bytes. Menor watermark interno registrado: 402111 bytes. Menor espaço de pilha registrado na tarefa da UI: 7828 bytes. Isso é uma regressão funcional, não um ensaio de estabilidade de 24 horas.

O áudio acumulou mais de 15439 blocos em uma sessão, sem erros reportados pelo codec. Não houve avaliação acústica humana. Pega-frutinha registrou aproximadamente 23–25 atualizações visuais/s sob consultas USB frequentes. **A meta de 30 fps ainda não foi atingida consistentemente nesse teste instrumentado.** A animação pixel art usa poses de 160 ms; contagem de quadros em repouso não representa capacidade máxima. Capturar e transferir BMP pela USB interrompe temporariamente a atualização da interface e reduz a métrica dessa janela. Valores brutos agregados em [runtime-metrics.json](runtime-metrics.json).

## Evidência visual

[Casa](screenshots/home.png), [oito pares](screenshots/memoria-oito-pares.png), [Jokenpô](screenshots/jogo-1.png), [resultado das frutas](screenshots/fruit-result.png), [A/B/C final](screenshots/abc.png), [retrô](screenshots/retro.png), [ficha](screenshots/info.png) e [álbum](screenshots/album.png). As folhas de coleção e cenários em previews são geradas pelo host; as imagens desta seção são capturas do Tab5.

## Testes reproduzíveis e limites

`tools/test_host.sh` passou com ASan/UBSan e warnings como erros: identidade, saves, corrupção, cuidados, sono/soneca, modos de tempo, relógio regressivo, fim de ciclo Clássico, 36 caminhos de evolução e jogos; herança em 500 sementes; protocolo com dois processos UDP. `tools/simulate.sh 123` executou 15 dias no mesmo núcleo. `tools/validate_assets.py` aprovou 179 arquivos, 54 bancos de formas/ovos, 48 WAVs e 62962687 bytes de conteúdo imutável. Resultados em host-tests.txt, simulation.txt e asset_coverage.md.

Ainda pendentes: audição humana, sensor touch por gesto manual, teste físico completo dos dez minutos da soneca (temporizador validado no host), autonomia, retenção do RTC sem alimentação, ensaio prolongado de cortes de energia/retirada de SD e integração LAN entre dois Tab5. O firmware de extras foi compilado; herança e protocolo foram testados no host, sem alegar integração física.

Os pacotes finais, seus tamanhos comprimidos e SHA-256 constam em dist/manifest.json e dist/SHA256SUMS. O pacote do cartão exclui progresso; para backup pessoal, copiar a pasta tamagotchi inteira. A pasta documental canônica é `Docs` para compatibilidade com sistemas que diferenciam maiúsculas de minúsculas.

## Revisão de interface e perfis

A/B/C foi removido na versão atual; as capturas anteriores desse modo são histórico da regressão inicial. Ajustes agora usa categorias e os submenus usam listas. A ficha foi conferida sem sobreposição. Nomes usam o teclado encaixável [A164, conforme documentação oficial](https://docs.m5stack.com/en/tab5/Tab5_Keyboard), sem teclado virtual. O protocolo I²C foi implementado de forma independente, em barramento separado do BSP. O acessório não respondeu à detecção durante esta sessão, portanto a digitação física continua pendente; a tela e seu salvamento foram exercitados.

Perfis: abrir, criar ovo em espaço vazio e apagar com confirmação e arquivo prévio do ciclo. Testes reais cobriram cancelamento, exclusão, reinício mantendo o espaço vazio e nova criação. Os saves do perfil de teste foram preservados em cópia privada e restaurados byte a byte. Capturas atuais: screenshots/ficha-touch.png, ajustes-touch.png, nome-fisico.png e perfil-vazio.png.

## Revisão 0.2.0 — 22/11/2026

Instalações incrementais pelo launcher 0.8.0-rc.2 no slot 2, com backup do aplicativo substituído no SD e cópias locais dos arquivos de saves antes de cada etapa. Na primeira ampliação necessária, o próprio launcher reorganizou sua reserva; nenhuma tabela ou bootloader deste build foi gravado. app0/Meshtastic e app1 anterior foram preservados, conferidos pelo inventário do launcher. O último binário padrão tem **931392 bytes**, SHA-256 `6e601f6547d57fee70bbe5cee24da964bef0176154e54d85f31bc7dc0802d65e`. Extras: **1269904 bytes**, compilado, sem teste entre dois aparelhos.

Resultados novos no Tab5:

- Abertura após migração de State v1 para v2 com mesma identidade, perfil e progresso; sem falha de save.
- Navegação repetida por casa, álbum, famílias, histórias, ficha do indivíduo e ajustes; capturas reais por USB.
- Apagamento configurado em 30 s; durante 65 s sem entrada, iluminação em zero, simulação avançando e sequência de checkpoint aumentando. Primeiro toque reacendeu sem abrir o botão sob ele; segundo toque navegou. Brilho e tempo originais restaurados.
- Nunca manteve a tela ligada durante 33 s. Pega-frutinha e três rodadas de esconde-esconde concluídos; interface e resultados exercitados.
- Medição sem capturas durante o jogo: aproximadamente 33,07 fps e 33,87 ciclos de UI/s nas janelas completas do Pega-frutinha. A primeira janela mista foi descartada. O erro encontrado de invalidação contínua da iluminação foi corrigido; métricas anteriores a essa correção não representam o build final.
- Contadores normais de falha do codec e de recursos de áudio em zero. Recursos de áudio e imagem foram corrompidos de forma reversível, mantendo os tamanhos: o áudio foi rejeitado por SHA-256 e a simulação continuou; recursos originais restaurados com leitura de volta e mesmo pet recuperado.
- O usuário confirmou manualmente teclado, touch e alguns reinícios aleatórios pelo botão. Esses relatos são distintos da entrada LVGL injetada nos ensaios automatizados.

Logs, respostas completas e backups permanecem em private/hardware. As métricas agregadas estão em runtime-metrics.json. Precisão da estimativa da bateria, autonomia, retenção do RTC, audição humana, cortes controlados em fases de escrita e visitas entre dois Tab5 não são considerados aprovados por esses ensaios.

O pacote com engine_api_version incompatível foi ensaiado no aparelho: abertura bloqueada e SHA-256 de todos os arquivos de saves idênticos antes/depois. Os recursos originais foram restaurados com leitura de volta, e o mesmo pet voltou sem erros de recursos. Durante os ensaios, o Wi-Fi do launcher entrou em falha transitória; um reinício via RTS, sem gravação de flash, restabeleceu a conexão. Não foi alterado o firmware do C6.

## Correção de teclado 0.2.1

Atualização pelo launcher no app2 com backup da revisão anterior. Teclado A164 detectado após substituir a sondagem simples pela leitura de identificação com timeout de 50 ms. Teste automatizado do handler HID no Tab5 verificou edição, símbolos, cursor, manutenção do rascunho após duas reconstruções, Enter e Esc; o nome original permaneceu salvo. O usuário confirmou digitação física correta na tela de mudar nome. Logs em private/hardware/keyboard-name-tests.*.
