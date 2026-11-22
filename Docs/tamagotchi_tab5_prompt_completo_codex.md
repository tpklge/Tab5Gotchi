# Prompt completo para o Codex — Bichinhos do Tab5

## 1. Missão e resultado esperado

Desenvolva e entregue um jogo completo de bichinho virtual para o M5Stack Tab5, inspirado na experiência dos Tamagotchis clássicos, com criaturas originais, interface colorida e todos os controles no touch.

Execute o trabalho no meu ambiente de desenvolvimento. Não entregue apenas um planejamento, um esqueleto, demonstrações isoladas ou instruções para eu terminar. Implemente o firmware, os sistemas do jogo, o conteúdo e TODOS os recursos necessários do microSD, execute os geradores e prepare os pacotes finais.

O binário deve conter somente o necessário à execução: código, drivers, renderização, áudio, touch, persistência e uma interface mínima de diagnóstico. Sprites, cenários, animações, fontes principais, ícones, efeitos sonoros, músicas, definições das criaturas e conteúdo volumoso devem estar no microSD, dentro de /tamagotchi.

Nome de trabalho: Bichinhos do Tab5.
Idioma da interface e documentação: português do Brasil.
Execução principal: offline, sem depender de conta, assinatura, API, servidor ou geração de conteúdo em tempo real.
Orientação: horizontal, com layout lógico de 1280 × 720.
Todo o conteúdo deve ter identidade original; não copie personagens, sprites, nomes comerciais, músicas ou sons oficiais do Tamagotchi.

O documento tamagotchi_tab5_requisitos_v0_1.md, se estiver disponível no workspace, é a referência de concepção. Sua indicação antiga de “somente requisitos” está superada por esta instrução de implementação. Este prompt é autossuficiente e resolve as decisões de produto necessárias para prosseguir.

## 2. Autonomia e limites de execução

- Leia os AGENTS.md aplicáveis antes de trabalhar.
- Faça escolhas rotineiras por conta própria e registre as decisões. Não interrompa para perguntar por nomes, cores, bibliotecas ou detalhes que possam ser resolvidos dentro desta especificação.
- Preserve minhas alterações locais, projetos existentes, imagens de firmware, saves e demais arquivos pessoais. Não faça limpeza abrangente do workspace.
- Se houver um projeto Tab5 indicado pelo contexto, trabalhe nele de modo isolado. Caso contrário, crie um diretório próprio chamado bichinhos-tab5 dentro da pasta de trabalho adequada.
- Não reduza o escopo para apenas uma criatura ou um minijogo. Use uma experiência vertical como marco intermediário e continue até a coleção e os recursos obrigatórios estarem completos.
- Se existir impedimento real, termine tudo que for independente dele, documente a evidência e identifique precisamente o que ficou bloqueado.
- Não invente testes de hardware, capturas de tela, métricas ou sucesso de compilação.
- Esta tarefa é desenvolver e empacotar. Não formate cartões, apague flash inteira ou substitua firmwares/launchers existentes sem autorização já fornecida na sessão. Prepare comandos corretos e artefatos revisáveis para a instalação quando não houver essa autorização.
- Se a tarefa atravessar várias sessões, mantenha progresso verificável em arquivos do projeto e retome os itens pendentes. Não trate um marco intermediário como conclusão do produto.

## 3. Descoberta do ambiente e aproveitamento dos projetos

Antes de escolher arquitetura e versões:
1. Identifique sistema operacional, toolchain, ESP-IDF, CMake, Python e ferramentas já instaladas.
2. Inspecione somente os projetos Tab5 relevantes disponíveis no workspace ou nos caminhos indicados pelo usuário. Reaproveite implementações que já funcionem para tela, touch, áudio, microSD, energia, RTC e launcher.
3. Identifique versão da placa, revisão do ESP32-P4 e controlador de tela/touch quando houver acesso ao hardware ou documentação do projeto.
4. Registre a origem e a licença do código reaproveitado.
5. Consulte documentação oficial M5Stack e Espressif para resolver lacunas, inclusive as revisões do Tab5.
6. Fixe versões reproduzíveis das dependências. Não escolha automaticamente a versão mais recente nem force uma versão antiga citada em outro projeto.
7. Faça um build básico do projeto de referência, quando existir, e registre seu resultado antes de alterar os drivers.

Referências:
- https://docs.m5stack.com/en/core/Tab5
- https://github.com/m5stack/M5Unified
- https://github.com/espressif/esp-idf

Preferência: ESP-IDF com C/C++ e bibliotecas compatíveis com o ambiente validado. É permitido usar componentes Arduino/M5Unified ou LVGL se isso aproveitar os drivers existentes sem comprometer a estabilidade e o tamanho. Justifique a escolha em docs/architecture.md.

Não transplante pinagens de outra placa ESP32-P4. Não suponha que toda revisão do Tab5 tenha o mesmo controlador ou a mesma inicialização.

## 4. Escopo funcional obrigatório

Entregue:
- Um bichinho ativo por perfil e suporte a três perfis locais independentes, com um perfil apresentado por vez.
- Seis famílias originais, cada uma com oito formas de vida e seu ovo.
- Sorteio persistente durante a incubação, características individuais e evolução influenciada pela convivência.
- Alimentação, água, banheiro, banho, sono, saúde, carinho, educação e exploração.
- Cenários coloridos específicos para as atividades.
- Oito minijogos completos.
- Modos tranquilo, contínuo e clássico, com tranquilo como padrão.
- Álbum de descobertas, diário, decoração, favoritos e retratos.
- Opção visual retrô e controles virtuais A/B/C, além dos controles touch diretos.
- Pacote completo de recursos no microSD, ferramentas de geração/conversão/validação e versão distribuível.
- Saves recuperáveis, diagnóstico de conteúdo e testes significativos.
- Controles de volume, efeitos, música, brilho e exibição discreta da bateria real.

Implemente também as ideias locais de ninar e reação suave à inclinação se o hardware permitir. As extensões de visitas por rede e reprodução/herança estão descritas na seção 22 e devem ser opcionais, isoladas e desativadas por padrão; não podem tornar o funcionamento offline dependente delas.

## 5. Interface e experiência

- Visual acolhedor, expressivo e legível em uma tela de cinco polegadas.
- Pixel art colorida consistente, com escala por vizinho mais próximo quando apropriado.
- Bichinho protagonista, suficientemente grande para ver suas expressões.
- Área principal para a cena; barra de informações discreta e painel de ações que não cubra permanentemente o animal.
- Botões com ícone e legenda. Comece com áreas touch de aproximadamente 80–110 pixels e ajuste pela validação real.
- Acesso simples a Alimentar, Higiene, Dormir, Carinho, Saúde, Jogos, Explorar, Ficha, Álbum e Configurações.
- Voltar, cancelar e fechar sempre acessíveis; não depender de teclado, mouse, gestos ocultos ou botões físicos.
- Feedback visual imediato e efeitos sonoros curtos.
- Entrada com controle de toque mantido, debounce e transformação correta entre coordenadas físicas e orientação horizontal.
- Gestos de banho e carinho sem ativar botões de outra camada.
- Bloquear múltiplas confirmações da mesma ação enquanto ela está sendo concluída, mantendo cancelamento permitido quando seguro.
- Pequenas animações de transição; nada de telas pretas prolongadas.
- Mostrar claramente a diferença entre energia do bichinho e bateria real do Tab5.
- Não estimar porcentagem precisa da bateria física apenas a partir de um valor de tensão. Se o hardware/API não fornecer uma estimativa confiável, exiba tensão/estado ou nível aproximado identificado como tal.
- O modo A/B/C deve ser uma alternativa funcional de navegação, sem retirar os botões diretos.
- O tema retrô altera a apresentação e mantém as mecânicas.

## 6. Famílias e coleção completa

Use os nomes abaixo como nomes de exibição. IDs e caminhos em ASCII minúsculo, estáveis e sem espaços.

Cada família tem exatamente:
1 bebê + 1 criança + 2 adolescentes + 3 adultos + 1 idoso = 8 formas.
Total: 48 formas distintas + 6 ovos temáticos.
Cores, manchas e acessórios não substituem formas de crescimento.

| Família/ID | Identidade | Bebê; criança | Adolescentes | Adultos | Idoso |
|---|---|---|---|---|---|
| Capim / capim | Capivara fantástica arredondada | Capipim; Capimirim | Capicalmo; Capicurioso | Capimanso; Capitrilha; Capifesta | Capivô |
| Brotito / brotito | Criatura vegetal com broto | Sementim; Brotinho | Folhanso; Ramisco | Florim; Galhote; Frutino | Tronquim |
| Brasquito / brasquito | Lagarto fantástico | Faisquito; Brasquim | Brasmanso; Brascout | Cristalume; Rocadraco; Faiscão | Brasancião |
| Gotim / gotim | Criatura aquática arredondada | Gotícula; Pinguim-d'água | Ondim; Saltágua | Coralino; Marulhão; Bolhudo | Gotavô |
| Nuvito / nuvito | Nuvem e penugem | Flocim; Nuvim | Brisanso; Ventito | Pluminho; Tempestino; Arcoíro | Cirrô |
| Parafim / parafim | Bichinho mecânico de brinquedo | Porquinha; Engrenim | Calmobô; Curiosô | Serenóide; Voltix; Ritmô | Ferruvô |

Alguns nomes são lúdicos, não descrições de espécies reais. Você pode corrigir uma colisão de nome ou inadequação evidente mantendo a identidade e registrando a alteração.

Direção visual:
- Capim: focinho, patas e proporções reconhecíveis de capivara fantástica; quintal pantaneiro e lago.
- Brotito: silhueta vegetal, folhas e crescimento visível do broto.
- Brasquito: cauda, pequenas escamas e traços de cristal/pedra/faísca nas formas adultas.
- Gotim: nadadeiras, bolhas ou detalhes de água, com suporte visual para cenas terrestres.
- Nuvito: corpo leve, nuvens/penas e motivos de clima.
- Parafim: juntas, peças e expressões de brinquedo mecânico; banho adaptado de forma coerente ao universo do jogo.

As três formas adultas de cada família devem apresentar diferenças de silhueta e detalhes, além da cor. O idoso é uma forma própria, com traços herdados visualmente do adulto anterior.

## 7. Incubação, identidade e aleatoriedade

Ao criar um ovo:
- Sorteie uniformemente entre as seis famílias habilitadas.
- Gere identificador único e semente usando a fonte de entropia adequada do dispositivo.
- Sorteie paleta, detalhes compatíveis, personalidade e preferências.
- Persista a identidade ANTES de iniciar a apresentação da incubação.
- Mantenha a família escondida até a eclosão; pequenas pistas visuais são permitidas.
- Use 3–5 minutos de incubação, escolhido de maneira persistente.
- Reiniciar, recarregar perfil ou fechar o aplicativo não altera o indivíduo.
- Incubação em modo tranquilo avança apenas com a simulação ativa.
- Não gere conteúdo por IA no dispositivo. As variações usam recursos previamente criados.
- Toda combinação habilitada deve possuir cobertura de sprites/animações ou uma composição válida nas etapas futuras.
- Trate nomes com teclado touch, limites razoáveis, UTF-8 e fonte com acentos.

Use geradores pseudoaleatórios separados para identidade/evolução, eventos e jogos. Jogar ou abrir menus não pode alterar retroativamente o nascimento ou um sorteio de evolução. Torne os geradores injetáveis para testes reproduzíveis.

## 8. Estados, atributos e cuidados

Atributos positivos de 0–100:
saciedade, felicidade, energia, higiene, saúde, vínculo e educação.

Necessidade de banheiro representa urgência crescente; rotule-a de maneira inequívoca.
Peso é fictício e normalizado por família/etapa; não use parâmetros veterinários reais.

Dados adicionais:
idade de simulação, etapa, forma, personalidade, preferências, hábitos, histórico de cuidados, doenças, objetos favoritos, horário de sono e tendências de evolução.

Mantenha estados explícitos para sono, doença, ação atual, animação e transições. Não derive todos os comportamentos apenas da tela aberta.

Implemente:
- Refeições, frutas, petiscos e água; ao menos 12 alimentos completos, com ícones e efeitos de dados.
- Sede simplificada por interação, sem nova barra obrigatória.
- Recusa quando satisfeito e proteção contra alimentação infinita.
- Petiscos melhoram humor, mas não substituem refeições.
- Carinho com resposta visual e ganho limitado de vínculo.
- Aviso de banheiro antes do acidente.
- Uso do vaso, descarga, acidente e limpeza.
- Banho com espuma, lavagem, enxágue e secagem.
- Sono, soneca, luz e horário ajustável.
- Ninar por carinho ou sequência musical curta.
- Orientar e elogiar; distinguir necessidade legítima de manha.
- Elogiar ou repreender repetidamente não aumenta educação indefinidamente.
- Doença leve com causa compreensível, tratamento e recuperação.
- Remédio sem necessidade não deve ser uma forma de ganhar atributos.
- Exploração curta do quintal com retorno, animação e eventos leves.
- Condições sempre recuperáveis no modo tranquilo, salvo despedida escolhida.
- Limites de ganho por ação e retorno decrescente, configuráveis em dados.

Faça balanceamento inicial jogável. Não deixe todos os parâmetros em zero ou valores demonstrativos.

## 9. Evolução e ciclo de vida

Tempos de referência, configuráveis:
- Bebê: nascimento até 2 horas de simulação.
- Criança: 2 horas até 24 horas.
- Adolescente: 24 até 72 horas.
- Adulto: 72 horas até 14 dias.
- Idoso: a partir de 14 dias.

Ovo não conta como idade pós-nascimento.

Evolução:
- Use tendências tranquilo, explorador e brincalhão/social.
- Some eventos de convivência em uma janela significativa da etapa, com limites por atividade para impedir spam.
- Considere personalidade, descanso, carinho, exploração, variedade de jogos e cuidados.
- Defina e documente pesos concretos, limiares, desempates e como cada forma adulta é escolhida.
- Ambos os adolescentes podem alcançar qualquer adulto da família, com tendência, sem bloqueio definitivo.
- Saúde e qualidade dos cuidados podem afetar progresso/traços, mas não transformam todos os adultos alternativos em punições.
- A forma resultante precisa ser persistida antes da animação, evitando novo sorteio ao reiniciar.
- Exiba pistas compreensíveis na ficha; mantenha surpresas no álbum.
- Evoluções durante ausência são registradas e apresentadas quando o usuário retornar.
- Implemente animação específica, jingle e entrada no diário.
- Forneça ferramenta de simulação acelerada para desenvolvimento; aceleração não deve ficar ativada numa compilação normal.

Uma sessão longa de recuperação temporal não pode sortear indevidamente várias formas ou perder as transições; defina a política e teste.

## 10. Tempo e modos de cuidado

Use relógio monotônico para intervalos enquanto o programa roda. Horário de calendário/RTC serve para rotina, apresentação e recuperação offline quando válido. Não suponha que RTC continuará correto após qualquer perda de alimentação.

### Tranquilo — padrão
- Sem envelhecimento ou desgaste enquanto o aplicativo estiver fechado ou o dispositivo desligado.
- Intervalos de suspensão são excluídos da idade de simulação.
- Ao voltar, retome sem punição pela ausência.
- Atualize dia/noite suavemente e trate rotina de sono sem fazer o bichinho colapsar pela mudança de horário.
- Sem morte por negligência e sem morte automática por velhice.
- Permita aposentadoria/despedida escolhida, com registro e novo ovo.

### Contínuo
- Necessidades e idade avançam durante ausência, com calendário válido.
- Tolerância inicial de 2 horas antes de consequências offline relevantes; evolução temporal pode continuar.
- Limite o desgaste offline ao equivalente a 12 horas por retomada.
- Preserve saúde mínima recuperável, sem morte offline nesse modo.
- Apresente um resumo do que aconteceu, sem uma avalanche de notificações.
- Torne esses parâmetros configuráveis e explique a regra ao jogador.

### Clássico
- Consequências mais rigorosas, com morte possível após negligência prolongada.
- Desativado por padrão, escolhido explicitamente na interface.
- Defina tempos de risco e recuperação em dados, com avisos claros.
- Mudança de relógio, erro de RTC, ausência do SD ou save corrompido não são causas de morte.
- Uma anomalia temporal limita a recuperação automática até resolução; não causa catástrofe.
- Histórico e álbum são mantidos após fim de vida.

Não recupere períodos enormes por um loop por segundo. Use integração por trechos/eventos e limites, com testes de meses de ausência e mudanças de data.

## 11. Cenários completos

Produza fundos, objetos e interações para:
1. Sala/casinha.
2. Mesa/cozinha.
3. Banheiro.
4. Área de banho, que pode compartilhar arquitetura visual com o banheiro.
5. Quarto.
6. Quintal/parque.
7. Cantinho de cuidados.
8. Salão de jogos.
9. Berçário.
10. Álbum e diário, com fundo/interface próprios.

Inclua variações de dia/noite onde fizer sentido, com camadas ou paletas.
Ao menos uma decoração temática por família.
Portas, objetos e botões devem indicar que são interativos.
Organize camadas para desenhar o animal atrás da mesa ou dentro da banheira corretamente.
Animações de necessidades corporais devem ser leves e cartunescas.
A leitura do cenário e dos objetos deve funcionar sem explicação técnica.

## 12. Oito minijogos completos

Todos precisam de instruções em português, entrada touch, rodada, pontuação, reações do bichinho, som, botão de saída e recompensa persistida corretamente.

### 1. Jokenpô
Pedra, papel e tesoura; disputa até três vitórias.
Empate repete a rodada.
A escolha do bichinho é definida e mantida antes de ler a do usuário.
Não adapte a jogada depois nem use a personalidade para trapacear.
O resultado de uma rodada não pode mudar por recarregar.

### 2. Par ou ímpar
Escolha paridade e número de 0 a 5.
Bichinho sorteia uniformemente 0 a 5 antes da confirmação.
Revele números, soma e resultado; zero é par.
Alterne quem escolhe a paridade entre rodadas.
Disputa de cinco rodadas; empate na pontuação geral é permitido.
Explique que são seis opções de número, não uma aposta com dinheiro.

### 3. Esquerda ou direita
Cinco tentativas para adivinhar para onde o bichinho olhará.
Escolhas com probabilidades iguais e animação de revelação.

### 4. Maior ou menor
Números de 1 a 9; escolher se o próximo será maior ou menor.
Número igual é empate e repete sem penalidade.
Ao sair de 1 ou 9, represente o caso corretamente; não introduza resposta impossível.
Escolhas do gerador independentes do toque.

### 5. Memória
Dificuldade inicial de seis pares, com opção de quatro ou oito pares.
Cartas com arte da coleção; embaralhamento válido, bloqueio de cliques durante resolução e identificação de pares.
Nenhuma dependência de ter desbloqueado a espécie no álbum.

### 6. Sequência musical
Quatro botões de cor/símbolo e notas.
Sequência crescente, tutorial simples e alternativa visual para jogar sem som.
Animação não deve aceitar entrada adiantada como se fosse resposta válida.

### 7. Pega-frutinha
Arrastar cesta ou personagem, rodadas de cerca de 30 segundos.
Frutas, animações, pontuação, colisão e dificuldade limitada.
Cadência de objetos não pode depender da taxa de frames.

### 8. Esconde-esconde
Bichinho se esconde em objetos válidos do cenário.
Três rodadas curtas, pistas visuais opcionais e revelação.
Não colocar o alvo fora da área tocável nem atrás de controles permanentes.

Regras comuns:
- Participação rende vínculo/felicidade moderados; vitória tem bônus limitado.
- Perder não causa doença.
- Respeite cansaço e sono; sugira descanso sem tornar todo acesso frustrante.
- Não use recompensas negativas grandes ou micropagamentos.
- Use IDs de partida/transação para evitar recompensas duplicadas por reboot.
- Salvar não deve prejudicar o áudio nem bloquear input por períodos perceptíveis.

## 13. Álbum, diário, decoração e apresentação

- Álbum de famílias e formas com retratos, silhuetas não descobertas e pistas.
- Registro de cada indivíduo, nome, aparência, nascimento, evoluções e despedida.
- Diário de eventos importantes, com limite e rotação de entradas.
- Brinquedo favorito visível na sala.
- Ao menos 12 objetos decorativos com desenhos próprios.
- Desbloqueio por convivência, descobertas e participação; não condicionar alimentação/remédio a moedas.
- Captura de retrato no SD, com sobreposição opcional de nome e data.
- Tema retrô com contraste legível.
- Reação suave à inclinação, se o sensor funcionar no projeto: apenas efeito estético, com opção de desligar e sem mudar aleatoriedade ou cuidados.
- Três perfis locais sem misturar saves, coleção individual e eventos.
- Configurações globais de som/brilho e opções de cuidado por perfil.

## 14. Recursos visuais — produzir e entregar os arquivos

Você é responsável por produzir os recursos. Não entregue pastas vazias, imagens transparentes, cópias do mesmo bichinho apenas recoloridas, emojis do sistema ou quadrados/círculos usados como substitutos do elenco.

Arte:
- Pixel art original, paleta coesa e fundos com composição intencional.
- Base sugerida de 96 × 96 por quadro de criatura, escalável sem borrão; ajuste se demonstrar necessidade.
- Âncoras e áreas de contato consistentes por ação, sem recortar caudas, folhas ou orelhas.
- Silhuetas distintas entre famílias e formas adultas.
- Pelo menos quatro paletas por família e duas variações de detalhe compatíveis com o ciclo.
- Não criar milhares de arquivos redundantes quando composição/atlas resolve; validar todas as combinações habilitadas.
- Reflexos/espelhamento apenas quando adequados; não espelhar texto ou acessórios de modo incoerente.

Cobertura de animações por forma:
idle, piscar, olhar, andar, feliz, triste, fome, cansaço, dormir, comer, beber, urgência de banheiro, usar banheiro, banho, doente, vitória, derrota e carinho.
Ovo: idle, balançar, rachadura e eclosão.
Evolução: sequência de transformação e efeitos.
Usar quadros compartilhados e overlays é permitido se a ação continuar visualmente clara.

Referência de quantidade:
- idle: 4–8 quadros.
- andar: 6 ou mais.
- comer/banho: 4 ou mais quadros específicos ou composição equivalente.
- demais reações: pelo menos 2–4 quadros ou composição expressiva com temporização documentada.
Não conte a repetição do mesmo quadro como animação completa.

Processo:
1. Se houver ferramenta de geração de imagens, use-a quando adequada para criar arte-base e fundos; depois normalize dimensões, transparência, paleta, âncoras e sequências.
2. Se essa ferramenta não existir, produza arte original com ferramentas disponíveis, incluindo desenho vetorial/pixel art e scripts de rasterização. O resultado precisa ser visualmente completo.
3. Se utilizar geração procedural offline, faça desenho por família e estágio; não se limite a trocar cor e tamanho de uma forma genérica.
4. Execute efetivamente geração e conversão.
5. Crie folhas de contato de todas as formas e prévias dos cenários.
6. Inspecione as imagens geradas com ferramenta de visualização, corrija recortes, repetição acidental, arte ilegível e inconsistências.
7. Exporte atlas/metadados e arquivos no formato realmente carregado pelo firmware.

Fontes devem incluir português com acentos, números e símbolos usados pela interface.
Inclua licença/proveniência dos recursos e preserve qualquer obrigação das fontes/bibliotecas utilizadas.

## 15. Sons e músicas — produzir e entregar os arquivos

- Efeitos originais para botões, confirmação, cancelamento, recusa, atenção, comida, água, banheiro, descarga, banho, sono, eclosão, evolução, tratamento e resultados dos jogos.
- Ao menos três vocalizações/assinaturas curtas por família: chamado, alegria e sono.
- Pelo menos oito temas/ambientes originais ou produzidos para o projeto, cobrindo casa, berçário, cozinha, banheiro/banho, quarto, parque, cuidados e jogos.
- Temas curtos de aproximadamente 20–45 segundos, com repetição suave e sem clique.
- Música do quarto discreta; opção de silêncio durante o sono.
- Ajustes separados de música, efeitos e volume geral; persistir configurações.
- Interromper/trocar música com transição curta sem cortar violentamente a saída.
- Vozes e efeitos simultâneos com mixagem limitada, proteção contra clipping e níveis consistentes.

Prefira inicialmente WAV PCM mono de 16 bits, por exemplo 22050 Hz, se compatível com a cadeia de áudio escolhida. Não inclua decodificadores pesados só para diminuir alguns megabytes no SD. Se outro formato for usado, documente custo e suporte.

A síntese por Python ou outra ferramenta offline é válida para criar chiptunes, ruídos, notas e vocalizações originais. Execute os scripts e entregue os WAVs; não deixe “gerar os sons” como tarefa do usuário.
Valide cabeçalhos, duração, amostras, níveis, loops e arquivos ausentes. Escute amostras representativas se houver ferramenta de áudio; caso contrário, informe essa limitação de validação, sem alegar audição realizada.

## 16. Pacote de recursos e esquema de dados

Estrutura de distribuição:
sdcard/tamagotchi/manifest.json
sdcard/tamagotchi/pets/
sdcard/tamagotchi/scenes/
sdcard/tamagotchi/audio/sfx/
sdcard/tamagotchi/audio/music/
sdcard/tamagotchi/ui/
sdcard/tamagotchi/data/
sdcard/tamagotchi/saves/
sdcard/tamagotchi/album/

A pasta física de montagem, como /sdcard, deve ser definida pelo BSP. O caminho relativo do conteúdo é sempre tamagotchi; não confunda ponto de montagem do firmware com caminho que o usuário copia à raiz do cartão.

Manifesto:
- schema_version, content_version e engine_api_version.
- Compatibilidade declarada, nomes dos pacotes e identificação das seis famílias.
- Arquivos, tamanhos e hashes de integridade.
- Resoluções, formatos, âncoras e definição das animações.
- Fontes, textos e índices de áudio.
- Identificadores estáveis e referências verificáveis.

Defina esquemas documentados para famílias, formas, evolução, alimentos, jogos, decoração e parâmetros.
Adicionar criatura que use mecânicas existentes deve ser principalmente uma mudança de conteúdo.
Mecânicas novas podem exigir firmware novo; não prometa atualização universal por JSON.

O pacote distribuído não contém meus saves. Diretórios mutáveis podem conter apenas instruções ou exemplos explicitamente separados.
Instalador/atualizador de recursos deve preservar saves e álbum; não use sincronização com exclusão abrangente.

## 17. Arquitetura e desempenho

Separe:
- board/HAL;
- modelo/simulação;
- ações e regras;
- evolução;
- tempo;
- persistência;
- conteúdo e validação;
- interface/cenas;
- animação;
- áudio;
- minijogos;
- coleção e extras.

O núcleo da simulação deve compilar no computador para testes e ferramentas de aceleração, compartilhando o mesmo código utilizado no ESP32.
Não duplique as regras num simulador independente que possa divergir do firmware.

Adote máquina de estados/eventos e relógios injetáveis.
Mantenha ações não bloqueantes; nada de sleeps longos na thread da interface.
SD e áudio têm acesso coordenado, com buffers e prioridades definidos.
Se usar LVGL, respeite a thread/mutex e ownership de buffers da versão escolhida.

Memória:
- Não carregue toda a coleção ou todos os fundos.
- Cache limitado por orçamento; liberar recursos ao trocar cenas/perfis.
- Streaming de música.
- Usar PSRAM onde os drivers permitirem e memória apropriada para DMA onde exigido.
- Referência: um frame RGB565 de 1280 × 720 ocupa 1.843.200 bytes, cerca de 1,76 MiB.
- Meça heap interna, PSRAM, maior bloco livre, pilhas e picos de cache.
- Evite crescimento de heap a cada visita ao menu ou partida.

Meta de apresentação: 30 fps nas cenas e jogos principais, resposta touch rápida e áudio sem falhas nas trocas de cena. Meça no hardware quando disponível; no host, apresente apenas métricas do host.
Não aumente clock ou use overclock para esconder gargalos.
Não prometa consumo ou autonomia antes de medir.

Firmware:
- Sem inclusão automática de PNGs, WAVs, atlas ou fundos como arrays C.
- Fonte/texto mínimo de diagnóstico pode ficar na flash.
- Verifique o mapa de link e reporte tamanho por componente.
- Compatibilidade com launcher/OTA existente deve ser inspecionada e preservada; não modificar partições por adivinhação.
- Recursos de rede opcionais devem poder ser excluídos da compilação para manter o binário mínimo.

## 18. Persistência robusta e falhas do SD

- Estado versionado, com ID do indivíduo/perfil, contador de sequência, integridade e compatibilidade.
- Salvar nascimento, evolução, ações relevantes, resultados e configurações.
- Checkpoint periódico, por exemplo a cada 60 segundos, ajustável.
- Informe a janela de perda possível para eventos não críticos.
- Estratégia de dois slots A/B verificados e cópia anterior, com flush/sincronização quando suportado.
- Não suponha que rename em FAT garante atomicidade após perda súbita de energia.
- Registrar uma ação crítica e seu identificador antes de apresentá-la como concluída.
- Permitir retomar ou cancelar ações interrompidas de maneira idempotente.
- Checkpoint compacto em NVS, limitado e com política de desgaste, pode complementar o SD; não gravar a cada frame.
- Não mesclar checkpoints de indivíduos ou perfis diferentes.
- Save mais recente inválido: tentar último válido; todos inválidos: informar e oferecer recuperação sem sobrescrever as evidências.
- Falta do SD: pausar simulação e mostrar instrução com a UI mínima.
- SD removido ou falha de leitura: cancelar I/O de maneira controlada, parar áudio com segurança e preservar o estado disponível.
- Erro de imagem isolada: fallback explícito, sem crash; reportar recurso ausente.
- Pacote incompatível: diagnóstico legível e bloqueio da simulação afetada, sem novo ovo automático.
- Atualização nunca substitui saves, álbum ou retratos do usuário.
- Exportação/importação e restauração de backup pela interface, com confirmação para substituição de progresso.
- Distinguir erro técnico de morte dentro do jogo.

## 19. Ferramentas de conteúdo e qualidade

Crie e execute:
- Gerador/preparador dos sprites, fundos, ícones, fontes e atlas.
- Gerador/sintetizador de efeitos e músicas.
- Conversor para os formatos de runtime.
- Validador completo do manifesto e referências.
- Relatório de cobertura por família × forma × animação.
- Verificação de colisões de IDs, nomes de arquivo, caminhos e arquivos órfãos.
- Verificação de imagens transparentes/vazias indevidas, quadros fora dos limites e animações com tempo inválido.
- Verificação de áudio ausente, vazio, com cabeçalho inválido ou saturação.
- Simulador de evolução com relógio controlado e sementes reproduzíveis.
- Ferramenta para empacotar firmware/documentação e sdcard.zip.
- Instalador/copiador de recursos com dry-run, verificação de destino e preservação de progresso.

Mantenha fontes editáveis e scripts reproduzíveis em assets_src/ e tools/, sem exigir que eles existam no microSD de uso.
Os arquivos finais devem vir prontos em sdcard/tamagotchi.
Não faça downloads em tempo de execução para completar recursos faltantes.

## 20. Testes e critérios verificáveis

Faça testes apropriados aos riscos reais, usando o mesmo núcleo da simulação. Evite testes que apenas repetem a implementação sem verificar comportamento.

Cubra ao menos:
1. Sorteio e persistência da identidade.
2. Reinício durante incubação e eclosão.
3. Cobertura das 48 formas e seis ovos.
4. Atingibilidade dos três adultos de cada família a partir dos dois adolescentes.
5. Limites dos atributos, recusa de alimento e cooldowns.
6. Banheiro, acidente, limpeza, banho e doença.
7. Sono, soneca e rotina configurável.
8. Contagem de idade no modo tranquilo.
9. Recuperação temporal limitada nos demais modos.
10. Relógio ausente/inválido, para trás e grande salto para frente.
11. Evolução registrada uma vez após retomada.
12. Jokenpô: matriz completa dos nove pares, empate e melhor de três.
13. Par/ímpar: combinações de 0–5, zero, soma e alternância.
14. Minijogos restantes: regras, cancelamento, pontuação e término.
15. Recompensa persistida uma única vez após reboot.
16. Separação dos perfis.
17. Save truncado, hash inválido, slot mais recente corrompido e queda em etapas de escrita.
18. SD ausente e pacote incompatível.
19. Recurso visual/sonoro ausente e reinicialização controlada.
20. Ciclo de troca de cenas/jogos sem vazamento relevante.
21. Atualização de conteúdo preservando progresso.
22. Carregamento de todos os recursos em formatos realmente suportados pelo firmware.

Testes de GUI no host ajudam, mas não provam touch, DMA, áudio ou SD no Tab5.
Faça testes no aparelho quando houver hardware e autorização de instalação; caso contrário, deixe um roteiro pronto e identifique essa validação como pendente.
Se possível, ofereça um preview desktop que reutilize regras e renderização/cenas; mantenha o firmware Tab5 como entrega principal.

## 21. Marcos de trabalho — avançar até a entrega integral

M1. Descoberta, arquitetura, BSP e build reproduzível.
M2. Uma família vertical com todos os cuidados, persistência e recursos reais.
M3. Oito minijogos completos e integração de áudio/cenários.
M4. Seis famílias, 48 formas, seis ovos e todas as animações.
M5. Perfis, álbum, diário, decoração, modos e recuperação.
M6. Inspeção visual, balanceamento inicial, testes, otimização e pacotes.
M7. Extensões opcionais da seção 22 com separação de build e documentação.

M2 não é o objetivo final. Não encerre a execução com “o restante pode ser adicionado depois” para requisitos obrigatórios.

A cada marco, atualize:
- docs/status.md: implementado, validado no host, compilado para o alvo, testado no hardware, pendências.
- docs/decisions.md: escolhas concretas e justificativas.
- docs/asset_coverage.md: recursos existentes e cobertura.
- docs/next_steps.md: tarefas objetivas para continuidade após interrupção.

Só marque algo como concluído se os arquivos e o comportamento existirem.

## 22. Extensões opcionais planejadas

Depois do núcleo obrigatório estar completo, implemente sem dependências externas desnecessárias:

### Reprodução/herança local
- Dois indivíduos adultos compatíveis de perfis diferentes podem originar um novo ovo por escolha do jogador.
- O ovo ocupa um perfil livre ou substitui um ciclo encerrado após confirmação.
- Família do descendente é uma das famílias parentais; não gerar híbridos que exigiriam arte inexistente.
- Herdar paleta/detalhes apenas entre combinações compatíveis, com pequena variação.
- Pais, descendente e evento ficam no álbum; não duplique ovos por recarregar.
- Não exigir reprodução para acessar toda a coleção.

### Visitas/troca de presentes em rede local
- Módulo opcional de build, desativado por padrão.
- Sem cloud, conta ou dependência para cuidar do pet.
- Dois dispositivos com versões compatíveis podem parear por confirmação/código e trocar um presente ou registro de visita.
- Limites de payload, timeout, validação de versão e IDs únicos para evitar duplicação.
- Não aceitar executáveis nem sobrescrever recursos/saves por mensagens.
- A criatura visitante é uma representação temporária; não transfira propriedade do save.
- Se só houver um dispositivo, teste o protocolo com dois processos no host, mas declare teste entre Tab5s como pendente.

Estas extensões são de prioridade posterior e isoláveis. Não adie nenhum recurso obrigatório para implementá-las. Se um impedimento de hardware bloquear a comprovação de uma extensão, entregue seu código/testes possíveis e informe o limite sem marcar a integração física como validada.

## 23. Entregáveis finais

1. Projeto-fonte completo, organizado e compilável.
2. Dependências fixadas e instruções de preparação reproduzíveis.
3. Recursos originais editáveis e ferramentas usadas para produzi-los.
4. Diretório sdcard/tamagotchi totalmente preenchido e validado.
5. Arquivo de distribuição do microSD, pronto para extrair na raiz do cartão.
6. Firmware de aplicativo e, se apropriado ao layout validado, imagem completa de instalação.
7. Arquivos de configuração de flash/partições, comandos e endereços obtidos do build real.
8. Checksums dos pacotes finais.
9. README em português: build, instalação, uso, cópia do SD, controles, recuperação e atualização sem apagar progresso.
10. Guia do usuário com criaturas, cuidados, jogos e modos.
11. Documentação de arquitetura, formatos e como adicionar uma família.
12. Prévias de todas as famílias e cenários, com folhas de contato.
13. Matriz de requisitos e evidências de validação.
14. Relatório real de tamanho do binário, tamanho do pacote, testes, consumo de RAM e desempenho medido quando possível.
15. Registro de licenças/proveniência.
16. Instruções de continuidade de qualquer item efetivamente bloqueado.

Não fixe endereços como 0x0/0x10000 sem verificar os artefatos do projeto. Rotule claramente imagem de aplicativo e imagem completa; respeite o launcher e o layout existente.

O pacote do SD não deve conter meu progresso pessoal. Não inclua diretórios de build desnecessários nos arquivos de distribuição.

## 24. Definição de pronto e resposta final

Considere pronto o núcleo quando:
- Existe firmware compilado para a revisão/alvo correto ou há um impedimento de toolchain claramente demonstrado, sem alegar compilação concluída.
- As seis famílias e todas as formas/ações têm recursos reais e válidos.
- Os oito jogos funcionam com regras, UI e resultados.
- As cenas e cuidados estão integrados ao mesmo save e simulação.
- Sons/músicas são arquivos existentes e utilizados.
- O validador do pacote não aponta referências obrigatórias ausentes.
- Os testes relevantes passam ou há uma limitação concreta identificada com evidência.
- Os pacotes finais foram gerados, inspecionados e possuem checksums.
- A documentação permite executar a instalação sem adivinhar pinagens ou endereços.

Falta de hardware não impede entregar o software e os recursos; impede somente alegar validação física.
Uma compilação aprovada não prova funcionamento no dispositivo.
Código que compila com funções vazias ou telas fictícias não satisfaz o jogo completo.
Arte e sons “a produzir” não contam como recursos entregues.

Na resposta final, entregue caminhos exatos dos pacotes e fontes, resumo do que funciona, comandos executados, resultados dos testes e limitações remanescentes. Dê destaque a qualquer diferença entre compilado, simulado e testado no Tab5.

Comece agora pela inspeção do workspace e do ambiente, estabeleça um plano operacional curto e avance na implementação, geração dos recursos, integração, validação e empacotamento até concluir o escopo.

