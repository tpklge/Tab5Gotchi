# Arquitetura

`main/core` contém as regras compartilhadas entre ESP32-P4 e testes C++ do host. Não usa LVGL, FreeRTOS, SD nem relógio do sistema. `tick` recebe tempo ativo e `resume` recebe calendário explícito. Identidade/evolução, eventos e jogos têm geradores separados com estado persistido; escolhas adversárias são preparadas antes do toque.

`main/storage` mantém dois slots por perfil, com cabeçalho, versão, comprimento e CRC32. A escrita só substitui o slot mais antigo, após gravar temporário, flush, fsync e reler. Ao carregar, vence a maior sequência válida. Os dois slots inválidos interrompem a simulação; nunca geram novo ovo. Arquivos pessoais são preservados pelo instalador.

`main/board` usa BSP oficial Tab5 1.3.2: inicialização dinâmica do painel/touch, rotação de 90 graus para 1280 × 720, SDMMC e codec. RTC RX8130CE é lido sem modificar a configuração da pilha. INA226 informa tensão real. Sensor BMI270 só desloca a apresentação. Sem rede no firmware padrão.

`main/ui` mantém toda chamada LVGL no mutex/thread do port. Eventos alteram o mesmo State testado no host. Há controles diretos e navegação A/B/C. O console USB permite inspeção, captura e injeção de ponteiro LVGL para regressões; ele não acelera o relógio normal do jogo.

Áudio executa em tarefa separada: buffers de 512 amostras mono, mixagem limitada de música, efeito e voz da família, saída estéreo 22050 Hz, fade de troca de tema, saturação em ±30000. Um mutex coordena leituras de áudio, recursos e saves. O cartão é obrigatório.

Orçamento de conteúdo: uma família/forma ativa com 32 poses RGB565+A8 (884736 bytes), um fundo RGB565 640 × 360 (460800 bytes), duas fontes A8 (~520 KiB), até oito miniaturas (221184 bytes), mais buffers de display e áudio. Não há imagens ou sons embutidos em arrays C. Fundos e sprites são ampliados por vizinho mais próximo.

Motor de evolução: idade ativa passa por 2 h, 24 h, 72 h e 14 dias. Carinho e ninar somam 2 pontos tranquilos, dormir 4, explorar 5 exploradores e partidas recompensadas 3 sociais. A personalidade soma 2 pontos na origem. Ao entrar na adolescência, o histórico anterior é reduzido a 1/4, dando relevância à convivência recente. Adolescente tranquilo se T >= E + S/2; os dois adolescentes podem atingir qualquer adulto. Adulto usa maior tendência, com desempate no RNG de identidade; idoso retém a variante adulta anterior. Recompensas do mesmo jogo têm intervalo de cinco minutos.

Não se afirma atomicidade absoluta no FAT durante remoção de energia. O formato binário 1 é little-endian e usa o layout fixo de State deste build; alterar a estrutura exige migrador e nova versão. O cabeçalho rejeita incompatibilidades de tamanho e preserva arquivos. Não habilitar migração por adivinhação.
