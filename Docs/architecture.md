# Arquitetura

`main/core` contém as regras compartilhadas entre ESP32-P4 e testes C++ do host. Não usa LVGL, FreeRTOS, SD nem relógio do sistema. `tick` recebe tempo ativo e `resume` recebe calendário explícito. Identidade/evolução, eventos e jogos têm geradores separados com estado persistido; escolhas adversárias são preparadas antes do toque.

`main/storage` mantém dois slots por perfil, com cabeçalho, versão, comprimento e CRC32. A escrita só substitui o slot mais antigo, após gravar temporário, flush, fsync e reler. Ao carregar, vence a maior sequência válida. Os dois slots inválidos interrompem a simulação; nunca geram novo ovo. Arquivos pessoais são preservados pelo instalador.

`main/board` usa BSP oficial Tab5 1.3.2: inicialização dinâmica do painel/touch, rotação de 90 graus para 1280 × 720, SDMMC e codec. RTC RX8130CE é lido sem modificar a configuração da pilha. INA226 informa tensão real. Sensor BMI270 só desloca a apresentação. Sem rede no firmware padrão.

`main/ui` mantém toda chamada LVGL no mutex/thread do port. Eventos alteram o mesmo State testado no host. A navegação usa touch direto; A/B/C foi retirado na revisão posterior de interface. O console USB permite inspeção, captura e injeção de ponteiro LVGL para regressões; ele não acelera o relógio normal do jogo.

Áudio executa em tarefa separada: buffers de 512 amostras mono, mixagem limitada de música, efeito e voz da família, saída estéreo 22050 Hz, fade de troca de tema, saturação em ±30000. Um mutex coordena leituras de áudio, recursos e saves. O cartão é obrigatório.

Orçamento de conteúdo: uma família/forma ativa com 32 poses RGB565+A8 (884736 bytes), um fundo RGB565 640 × 360 (460800 bytes), duas fontes A8 (~520 KiB), até oito miniaturas (221184 bytes), mais buffers de display e áudio. Não há imagens ou sons embutidos em arrays C. Fundos e sprites são ampliados por vizinho mais próximo.

Motor de evolução: idade ativa passa por 2 h, 24 h, 72 h e 14 dias. Carinho e ninar somam 2 pontos tranquilos, dormir 4, explorar 5 exploradores e partidas recompensadas 3 sociais. A personalidade soma 2 pontos na origem. Ao entrar na adolescência, o histórico anterior é reduzido a 1/4, dando relevância à convivência recente. Adolescente tranquilo se T >= E + S/2; os dois adolescentes podem atingir qualquer adulto. Adulto usa maior tendência, com desempate no RNG de identidade; idoso retém a variante adulta anterior. Recompensas do mesmo jogo têm intervalo de cinco minutos.

Não se afirma atomicidade absoluta no FAT durante remoção de energia. O payload atual é State versão 2, little-endian. O layout v1 de 4872 bytes está preservado explicitamente em storage/legacy_state.hpp, com migração validada por CRC, identidade e valores. Saves e arquivos de ciclos v1 continuam legíveis; novas gravações usam v2. Datas antigas não disponíveis ficam desconhecidas. Há backup A/B e exportação pelo launcher antes das atualizações físicas.

## Revisão 0.2.0

O renderer carrega arquivo, dimensões, número de quadros, âncora, período e sequências de cada forma no JSON. Limite de 192×192, 128 poses e 4 MiB por banco. O pet só invalida sua imagem quando muda a pose, mantendo movimentação independente. Miniaturas conservam oito buffers de um quadro; a integridade do banco é conferida no primeiro carregamento, e bancos ativos são verificados ao carregar. Leituras completas de recursos confrontam SHA-256 com o manifesto; áudio verifica o arquivo antes de abri-lo, em sua tarefa e sob o mutex de I/O. Erros de áudio têm contador separado e caminho no log USB. Ausência de recurso não cria outro pet.

As sequências completas de comer, banho, olhar, cansaço, vitória, derrota e transformação são usadas. Silhuetas preservam alpha e usam cinza uniforme. O esconde-esconde tem seis objetos originais e revelação do pet. O painel de ações tem fade de 160 ms; dia/noite atualiza continuamente, com aurora 05:30–06:30 e entardecer 17:30–18:30. A preferência posterior por controles cinza, teclado encaixável e ausência de A/B/C foi mantida.

Regras de ganhos, cooldowns, durações, recompensa e checkpoint vêm de data/rules.json. Cada atividade pode contribuir até 40 pontos de evolução por etapa; o orçamento renova ao mudar de etapa. Boa saúde (>=70) e higiene (>=50) multiplicam a contribuição por 1,25, arredondada para inteiro. Participar de um tipo de jogo novo na etapa acrescenta dois pontos à contribuição social, respeitando o limite individual. Pet/carinho, ninar, elogio e orientação têm retorno de atributos decrescente conforme seu orçamento é consumido, até o piso configurado de 25%. Necessidades essenciais permanecem recuperáveis. Pesos de base e a escolha por maior tendência/desempate persistido continuam documentados acima.

Exploração sorteia uma descoberta ao terminar, usando apenas eventRng; não altera os sorteios de jogos/identidade. State v2 acrescenta datas conhecidas de criação/nascimento/fim, marcos por etapa, pais, tipos de jogos conhecidos e contribuições por atividade. Histórias permite abrir o indivíduo atual ou arquivos life-/deleted- do perfil, com aparência, datas, diário e parentesco. O histórico circular continua limitado a 40 eventos; os seis marcos são independentes dele.
