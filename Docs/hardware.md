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
