# Bichinhos do Tab5 — levantamento de requisitos e proposta de jogo

Versão: 0.1 — 22/11/2026
Estado: documento de concepção. Nenhum firmware, código do jogo, sprite ou som foi implementado.
Nome de trabalho: Bichinhos do Tab5. O nome definitivo permanece em aberto.

## 1. O que o usuário já definiu

Estes itens são requisitos confirmados, e não apenas sugestões:

| ID | Requisito |
|---|---|
| U01 | Desenvolver um bichinho virtual inspirado na experiência do Tamagotchi para o M5Stack Tab5. |
| U02 | Usar a tela touch para todos os botões, cuidados e jogos; não exigir teclado externo. |
| U03 | Manter no binário apenas o necessário ao funcionamento do programa. |
| U04 | Armazenar sons, sprites e demais recursos volumosos no microSD, sob /tamagotchi. |
| U05 | Criar bichinhos novos, com identidade visual própria. |
| U06 | Ter nascimento aleatório durante a incubação do ovo. |
| U07 | Dar a cada tipo de bichinho sua própria linha de evolução. |
| U08 | Usar cores e cenários relacionados à ação: banheiro para necessidades e banho, mesa para alimentação etc. |
| U09 | Incluir minijogos com o próprio bichinho, começando por pedra-papel-tesoura e par ou ímpar. |
| U10 | Nesta etapa, apenas pesquisar, anotar requisitos, propor evoluções e apresentar ideias. |

Todas as quantidades, nomes, tempos, probabilidades, modos e detalhes técnicos abaixo são propostas para discussão. Não estão implicitamente aprovados por este documento.

## 2. Referência: Tamagotchi original

Referência histórica: a experiência das primeiras gerações P1/GEN1 e P2/GEN2, consultada nos manuais e materiais oficiais atuais da linha Original. As reedições não devem ser tratadas como reprodução comprovadamente idêntica de todos os tempos e algoritmos de 1996/1997.

O núcleo documentado inclui:
- Refeição e petisco; indicadores de saciedade e felicidade.
- Sono e controle da luz.
- Fezes na tela e limpeza.
- Doença e medicamento.
- Disciplina e chamados de atenção.
- Consulta de idade e peso.
- Três botões: selecionar, confirmar e cancelar.
- Ovo, crescimento e forma adulta influenciada pelo cuidado.
- GEN1: adivinhar a direção; GEN2: adivinhar se o próximo número será maior ou menor.
- Fim de vida e possibilidade de iniciar outro ovo.

A FAQ oficial informa uma criatura ativa por vez, incubação de aproximadamente cinco minutos e diferenças de personagens entre gerações. Acessórios, banho com cenário, carinho por toque, vários minijogos e catálogo persistente são expansões propostas para nosso jogo; não são atribuídos ao aparelho original.

Fontes históricas: [F1]–[F5].

## 3. Conceito proposto

Um companheiro virtual colorido, offline, com animações expressivas e cuidados em sessões curtas. O jogador acompanha uma vida, descobre uma evolução e começa uma nova geração quando desejar encerrar o ciclo.

Proposta de regra central:
1. A família biológica é sorteada ao criar o ovo.
2. Preferências e pequenos traços individuais também são sorteados.
3. O estilo de cuidado influencia a forma adolescente e adulta.
4. Saúde, aparência e humor contam a história das interações.
5. O álbum preserva os bichinhos já criados.

Padrão sugerido: um bichinho ativo. Vários perfis independentes podem ser uma expansão. Ter vários simultâneos muda a experiência de cuidado e não é requisito confirmado.

## 4. Cuidados, indicadores e consequências

Escalas internas sugeridas: 0–100, com nomes exibidos de modo consistente. Valores maiores significam melhor condição nos indicadores positivos.

| Indicador | O que representa | Ações relacionadas |
|---|---|---|
| Saciedade | Alimentação suficiente | Refeições; petiscos com efeito menor |
| Felicidade | Humor e diversão | Jogos, carinho, atividades favoritas |
| Energia | Disposição física | Sono e descanso |
| Higiene | Limpeza corporal | Banho, limpeza do ambiente |
| Saúde | Condição geral | Rotina equilibrada, tratamento quando indicado |
| Vínculo | Confiança no cuidador | Carinho e atenção regular |
| Educação | Hábitos aprendidos | Orientação, elogio e rotina |
| Necessidade de banheiro | Urgência fisiológica | Usar o banheiro; limpar acidentes |

Idade, peso fictício, personalidade, preferências e histórico ficam na ficha detalhada. Não transformar todos os indicadores em barras permanentes na tela principal.

### Alimentação
- Refeição, fruta e petisco com porções e animações próprias.
- Favoritos e alimentos menos apreciados por indivíduo, sem obrigar tentativa infinita.
- Petisco ajuda o humor, mas não substitui uma alimentação equilibrada.
- Recusa quando satisfeito; evitar alimentar indefinidamente.
- Incluir água como interação ou necessidade simplificada; sede separada fica pendente.
- Comidas e efeitos definidos em arquivos de dados no microSD.

### Banheiro e banho
- Expressão de urgência antes do acidente.
- Levar ao banheiro durante a janela de aviso.
- Se ocorrer acidente, limpar o ambiente; o bichinho continua recuperável.
- Banho separado de descarga: ambos podem usar o cenário de banheiro.
- Lavar com espuma e enxaguar por toques/arrastos simples.
- Necessidades e alimentação continuam com simulação coerente durante ações longas.

### Sono e educação
- Quarto com luz e ciclo de dia/noite.
- Horário de sono ajustável à rotina do usuário.
- Não interromper o sono com avisos não urgentes.
- Educação baseada em orientar e elogiar, preservando o espírito do original.
- Antes de permitir uma bronca, distinguir uma necessidade real de uma chamada sem necessidade.
- Repetir elogios ou broncas não deve aumentar atributos indefinidamente.
- Ações indevidas podem afetar o vínculo; oferecer explicação compreensível.

### Saúde
- Estados de indisposição, doença leve e recuperação, sem excesso de complexidade.
- Mostrar o motivo provável: sujeira acumulada, privação de sono ou alimentação inadequada.
- Remédio só quando houver indicação no jogo; não tratar medicação como alimento.
- Evitar doenças puramente aleatórias e inevitáveis.
- Saúde baixa deve permitir recuperação; ausência de alguns cuidados não deve bloquear todas as evoluções interessantes.

## 5. Nascimento aleatório e identidade

Proposta inicial de seis famílias, com chances iguais entre as famílias instaladas e habilitadas. Não incluir famílias raras na primeira versão; isso facilita descobrir a coleção sem frustração.

No início da incubação, sortear e salvar:
- Identificador único, semente aleatória e família.
- Paleta entre opções desenhadas para essa família.
- Detalhes compatíveis: manchas, olhos, orelhas, pequenos apêndices ou acessórios naturais.
- Personalidade inicial e preferências de comida/brincadeira.

A espécie permanece oculta até a eclosão. O ovo pode apresentar pequenas pistas de cor ou movimento.

Reiniciar, sair do jogo ou recarregar o save não sorteia outro indivíduo. A semente e as escolhas são persistidas antes de mostrar a incubação.

A aparência deve ser combinatória dentro de peças e paletas preparadas. Não pressupor geração de imagens por IA em tempo real no Tab5, nem gerar combinações sem sprites compatíveis em todas as etapas.

## 6. Famílias originais propostas

Nomes provisórios, disponíveis para alteração.

| Família | Identidade | Ambiente favorito | Três propostas de formas adultas |
|---|---|---|---|
| Capim | Criatura redonda inspirada em capivara | Quintal com lago | Capimanso, Capitrilha, Capifesta |
| Brotito | Criatura vegetal com broto na cabeça | Jardim | Florim, Galhote, Frutino |
| Brasquito | Pequeno lagarto fantástico | Pedras e jardim ensolarado | Cristalume, Rocadraco, Faiscão |
| Gotim | Criatura aquática de corpo arredondado | Aquário-jardim | Coralino, Marulhão, Bolhudo |
| Nuvito | Criatura de nuvem e penugem | Jardim nas nuvens | Pluminho, Tempestino, Arcoíro |
| Parafim | Criatura mecânica de brinquedo | Oficina aconchegante | Serenóide, Voltix, Ritmô |

As famílias compartilham os sistemas de cuidado, mas têm desenhos, sons curtos, preferências e animações distintas. Adaptações visuais permitem todas usarem as cenas principais: Gotim pode ter cadeira-aquário; Parafim usa um banho lúdico adequado ao seu universo.

Evitar depender de nomes, personagens, sprites ou sons oficiais do Tamagotchi.

## 7. Ciclo de vida e árvore de evolução

Proposta por família:
- 1 bebê.
- 1 forma criança.
- 2 formas adolescentes.
- 3 formas adultas.
- 1 forma idosa, com detalhes da forma adulta anterior.

São oito formas por família, além do ovo. Com seis famílias: 48 formas + seis ovos temáticos. Cores e variações por peças não contam como novas formas. Esse total é uma proposta de coleção completa, não uma exigência para o primeiro protótipo.

### Exemplo completo: família Capim

| Etapa | Forma | Regra proposta |
|---|---|---|
| Ovo | Ovo do Capim | Família sorteada e já persistida |
| Bebê | Capipim | Forma inicial comum à família |
| Criança | Capimirim | Crescimento após a fase de bebê |
| Adolescente | Capicalmo | Mais descanso, carinho e atividades tranquilas |
| Adolescente | Capicurioso | Mais brincadeiras variadas e exploração |
| Adulto | Capimanso | Predomínio de calma, vínculo e rotina |
| Adulto | Capitrilha | Predomínio de exploração e atividade |
| Adulto | Capifesta | Predomínio de sociabilidade e variedade de brincadeiras |
| Idoso | Capivô | Aparência com traços do adulto de origem |

Os dois adolescentes podem chegar aos três adultos. O adolescente cria uma tendência; não bloqueia permanentemente as alternativas.

### Como escolher a evolução
- Avaliar o histórico de uma janela relevante da etapa, e não apenas o último clique.
- Considerar cuidados, variedade das atividades e personalidade individual.
- Propor três tendências: tranquilo, explorador e brincalhão/social.
- Qualidade do cuidado afeta saúde, confiança e detalhes estéticos, sem reduzir toda a árvore a uma classificação de “bom” ou “ruim”.
- Uma pequena influência aleatória pode resolver empates; o resultado deve ser salvo e não mudar ao reiniciar.
- Uma ficha de pistas explica o caminho sem revelar todas as surpresas.
- Repetir o mesmo minijogo centenas de vezes não deve garantir evolução imediata.
- Definir equações, limites e pesos somente na etapa de balanceamento.

### Tempos sugeridos, para balancear depois

| Etapa | Faixa aproximada de idade no jogo |
|---|---|
| Ovo | Incubação de 3–5 minutos |
| Bebê | Do nascimento até aproximadamente 2 horas |
| Criança | De 2 horas até 1 dia |
| Adolescente | Do dia 1 ao dia 3 |
| Adulto | Do dia 3 ao dia 14 |
| Idoso | A partir do dia 14, sem despedida forçada no modo tranquilo |

Idade da tabela significa tempo de simulação acumulado; a passagem do tempo com o aparelho desligado depende do modo escolhido. Os intervalos são sugestões, não dados do Tamagotchi original.

Evolução durante o sono ou ausência deve ficar registrada. A apresentação visual é mostrada na próxima oportunidade, sem perder a animação nem exigir que o usuário esteja presente no segundo exato.

## 8. Cenários e ações no touch

Orientação horizontal proposta: 1280 × 720, compatível com o uso da tela do Tab5 nessa orientação. A documentação do Tab5 confirma tela touch, microSD, áudio e RTC [F6].

| Cena | Ações e elementos |
|---|---|
| Sala/casinha | Bichinho livre, carinho, brinquedos e acesso aos cuidados |
| Mesa/cozinha | Seleção de prato, mastigação, reação ao alimento e água |
| Banheiro | Vaso, descarga e limpeza de acidentes |
| Área de banho | Banheira, espuma, enxágue e secagem |
| Quarto | Cama, luz, soneca e sono noturno |
| Quintal/parque | Exercícios, exploração e jogos ativos |
| Cantinho de cuidados | Exame lúdico, descanso e tratamento |
| Salão de jogos | Mesa de jokenpô, par ou ímpar e outros jogos |
| Berçário | Ovo, incubação, eclosão e escolha de nome |
| Álbum | Coleção, retratos e histórico dos indivíduos |

Banheiro e banho podem compartilhar um fundo com diferentes objetos e enquadramentos. As famílias podem ter variações decorativas sem exigir um conjunto completamente distinto de fundos para cada espécie.

### Interface
- Barra de estado discreta: nome, idade/etapa, horário e condição urgente.
- Botões touch grandes com ícones e legenda em português.
- Menu de cuidados com alimentação, higiene, sono, carinho, saúde, jogos e ficha.
- Ações contextuais ao tocar no próprio bichinho ou nos objetos da cena.
- Voltar e cancelar sempre acessíveis.
- Modo opcional com três botões virtuais A/B/C para nostalgia.
- Feedback visual e sonoro breve; volume, música e efeitos ajustáveis separadamente.
- Diferenciar claramente energia do bichinho e bateria real do Tab5.
- Adotar provisoriamente alvos touch de aproximadamente 80–110 pixels e validar no aparelho.
- Evitar menus profundos, textos pequenos e avisos repetitivos.

### Animações mínimas por forma
Espera, piscar, olhar, andar, feliz, triste, fome, cansado, dormir, comer, beber, urgência, banheiro, banho, doente e reação de vitória/derrota. Algumas ações podem compartilhar quadros ou efeitos sobrepostos. Criar uma matriz de cobertura dos recursos antes de expandir o elenco.

## 9. Minijogos

“Jokenpô” é pedra-papel-tesoura. Os dois jogos pedidos pelo usuário entram no escopo confirmado; os demais são propostas.

| Jogo | Interação | Regras sugeridas |
|---|---|---|
| Jokenpô | Três botões com pedra, papel e tesoura | Melhor de três vitórias; empate repete; bichinho escolhe antes de conhecer a jogada do usuário |
| Par ou ímpar | Escolher paridade e tocar um número de 0–5 | Bichinho sorteia 0–5; somar e revelar; zero é par; alternar quem escolhe a paridade |
| Esquerda ou direita | Dois botões | Adivinhar para onde o bichinho vai olhar; homenagem ao GEN1 |
| Maior ou menor | Dois botões | Adivinhar o próximo número; tratar igualdade como empate explícito |
| Memória | Tocar cartas | Poucos pares com desenhos da coleção |
| Sequência musical | Repetir botões iluminados | Sequências curtas com notas e cores |
| Pega-frutinha | Arrastar cesta | Capturar frutas, com rodada curta |
| Esconde-esconde | Tocar objetos do cenário | Procurar o bichinho com pistas |

Requisitos comuns:
- Regras rápidas e botão de saída.
- Rodadas curtas, com duração e dificuldade ajustáveis.
- Aleatoriedade justa; em jogos de sorte, a personalidade não muda secretamente as probabilidades.
- Felicidade e vínculo por participação; bônus moderado por vitória.
- Perder não causa doença nem penalidade grande.
- Cansaço por sessões longas, com sugestão de descanso.
- Histórico de vitórias opcional.
- Moedas apenas como expansão; cuidados básicos nunca dependem de vencer apostas ou gastar moeda.

## 10. Tempo, ausência e fim de vida

Este ponto exige uma decisão de produto porque um tablet de desenvolvimento pode ficar desligado por dias.

### Modo tranquilo — padrão sugerido
- Ao fechar o aplicativo ou desligar, suspender necessidades e envelhecimento.
- Ao reabrir, retomar sem punição pela ausência.
- Horário real ainda organiza a aparência de dia/noite e a rotina de sono, com transição de retomada.
- Idoso pode permanecer vivo até o jogador escolher uma despedida/aposentadoria.

### Modo contínuo — opcional
- Calcular o tempo decorrido usando horário persistido e RTC, quando válido.
- Necessidades evoluem enquanto o aplicativo não está aberto.
- Período de tolerância e limite de deterioração após longas ausências; valor inicial sugerido de 12 horas a validar.
- Não executar um loop por segundo para recuperar meses de ausência.
- Tratar relógio inválido, salto para trás e grande salto para frente sem matar, rejuvenescer ou duplicar recompensas.
- A confiabilidade do RTC desligado deve ser verificada no hardware real, inclusive alimentação de retenção.

### Modo clássico — opcional
- Permitir consequências mais rigorosas e morte por negligência prolongada.
- Explicar o modo antes da escolha.
- Preservar nome, retrato, árvore alcançada e histórico no álbum.
- Novo ovo não apaga o histórico do anterior.

Separar fim de ciclo escolhido pelo jogador de morte por descuido. Não ativar morte ou reprodução automaticamente como se já fossem requisitos aprovados.

## 11. Conteúdo no microSD e binário mínimo

Diretório obrigatório: /tamagotchi.

Organização proposta:

| Caminho | Conteúdo |
|---|---|
| /tamagotchi/manifest.json | Versão do pacote, compatibilidade e relação dos recursos |
| /tamagotchi/pets/ | Definições das famílias, etapas, sprites e animações |
| /tamagotchi/scenes/ | Fundos, camadas, objetos e decoração |
| /tamagotchi/audio/sfx/ | Efeitos curtos |
| /tamagotchi/audio/music/ | Músicas e ambientes |
| /tamagotchi/ui/ | Ícones, botões e fontes do jogo |
| /tamagotchi/data/ | Comidas, itens, parâmetros e tabelas de evolução |
| /tamagotchi/saves/ | Estado atual, perfis e salvamentos anteriores |
| /tamagotchi/album/ | Coleção e histórico de gerações |

### Dentro do firmware
Motor de cuidados, regras e validação dos dados, renderização, áudio, touch, acesso ao SD, persistência, carregamento de conteúdo e uma tela mínima de recuperação com texto legível. A necessidade de suporte a launcher/OTA deve ser definida após verificar o ambiente existente do Tab5; não pressupor um formato de empacotamento.

### Fora do firmware
Sprites, atlas, fundos, músicas, efeitos, fontes principais, ícones, paletas e conteúdo declarativo. Adicionar uma espécie deve ser possível por pacote no SD quando usar mecânicas já suportadas. Uma mecânica nova ainda pode exigir firmware novo.

Não prometer um tamanho final do binário antes de escolher bibliotecas e medir a compilação. O objetivo é evitar recursos volumosos embutidos, mantendo o funcionamento básico de diagnóstico.

### Carregamento e desempenho
- Carregar somente a cena e a família/etapa necessárias, com cache limitado.
- Fazer streaming de música; evitar carregar a biblioteca inteira em RAM.
- Avaliar PNG versus atlas pré-convertidos RGB565/alpha com base em desempenho e custo de memória reais.
- Uma imagem RGB565 de 1280 × 720 ocupa cerca de 1,76 MiB; duas ocupam cerca de 3,52 MiB, antes de caches, sprites e áudio.
- Meta inicial de interface e animações fluidas a 30 fps, sujeita a medição; não é resultado já demonstrado.
- Verificar a revisão concreta do Tab5, seus drivers e o framework existente antes de fixar ESP-IDF, LVGL ou outra biblioteca.
- Wi-Fi não é necessário para cuidar do bichinho. Atualização de pacote pela rede é expansão opcional.

## 12. Salvamento e recuperação

- Salvar após cuidados relevantes, nascimento, evolução e alterações de configuração.
- Checkpoints periódicos com intervalo a definir; evitar gravar a cada frame.
- Gravação temporária + validação + substituição, com cópia anterior recuperável e número de sequência.
- Incluir versão do formato, verificação de integridade e migração documentada.
- Persistir semente, identidade, tendências de evolução, horários, inventário e estado de ações importantes.
- Operações aplicadas uma única vez, sem duplicar refeição ou recompensa após reinício.
- Reservar opcionalmente um checkpoint compacto em NVS interna, para recuperação; isso não significa guardar sprites ou sons no binário.
- Ausência/remoção do SD pausa a simulação, apresenta mensagem legível e não inicia um novo bichinho automaticamente.
- Recurso ausente ou pacote incompatível não apaga o save; permitir voltar ou corrigir o cartão.
- Atualizar conteúdo não pode substituir /saves nem /album.
- Exportação e importação manual do save como ferramenta de manutenção.
- Desligamento sem bateria não deve transformar corrupção de arquivo em morte dentro do jogo.

## 13. Ideias adicionais

Propostas que combinam com o conceito:
- Álbum de descobertas com silhuetas das formas ainda não vistas e pistas de evolução.
- Personalidade visível em pequenas reações e comportamentos espontâneos.
- Brinquedo favorito deixado na sala e pequenos hábitos individuais.
- Decoração conquistada por vínculo, tempo de convivência e participação.
- Diário com nascimento, primeira vitória, evolução e momentos engraçados.
- Botão de retrato que salva uma imagem do bichinho.
- Tema de quintal pantaneiro para Capim e varanda com rede.
- Modo visual retrô monocromático, mantendo a interface touch.
- Ninar tocando uma sequência ou acariciando com o dedo.
- Reações suaves à inclinação do Tab5, apenas depois de validar o sensor e sem substituir os botões.
- Visitas e troca de presentes entre dispositivos como expansão posterior, sem depender de rede para a experiência principal.
- Reprodução e herança visual apenas numa etapa futura, após o ciclo individual estar equilibrado.

## 14. Escopo sugerido por etapas

Esta divisão organiza trabalho futuro; nenhuma etapa é autorizada para implementação neste momento.

1. Experiência vertical de validação: uma família completa, nascimento, cuidados, cenários essenciais, saves e os dois jogos solicitados.
2. Primeira versão ampliada: três famílias, álbum, rotina de sono, saúde, educação e mais minijogos.
3. Coleção completa proposta: seis famílias e 48 formas, variações visuais, decoração e melhorias de apresentação.
4. Expansões opcionais: múltiplos perfis, visitas, herança e recursos de rede.

Critério principal: uma família deve ter cobertura de todas as ações, sons e etapas antes de multiplicar o elenco.

## 15. Critérios de aceitação para orientar o desenvolvimento futuro

| ID | Comportamento esperado |
|---|---|
| A01 | Todo o ciclo básico pode ser operado apenas pelo touch. |
| A02 | Reiniciar durante a incubação mantém o mesmo ovo e indivíduo sorteado. |
| A03 | Famílias diferentes têm linhas e aparências de evolução próprias. |
| A04 | Alimentação, banheiro, banho, sono e jogos usam cenários coerentes. |
| A05 | Jokenpô e par ou ímpar não escolhem a resposta após ler a jogada do usuário. |
| A06 | Áudio e animação não bloqueiam o cancelamento e a navegação. |
| A07 | Reiniciar após alimentar, jogar ou evoluir não duplica efeitos nem recompensas. |
| A08 | Reiniciar ou perder energia recupera o último estado válido, dentro da janela de salvamento documentada. |
| A09 | Iniciar sem SD mostra diagnóstico e preserva o bichinho existente. |
| A10 | Recursos volumosos ficam no SD e há orçamento de RAM/flash medido no aparelho. |
| A11 | Alterar a data ou voltar depois de longa ausência segue explicitamente o modo de tempo escolhido. |
| A12 | Trocar pacote de recursos não apaga saves nem coleção. |

## 16. Decisões pendentes e padrão proposto

| Tema | Padrão sugerido, ainda não aprovado |
|---|---|
| Nome do jogo | Bichinhos do Tab5, provisório |
| Aparência | Pixel art colorida expressiva; tamanho dos sprites a definir |
| Quantidade inicial | Três famílias na versão ampliada; seis como coleção completa |
| Tempo com aparelho desligado | Pausado, com modo contínuo opcional |
| Morte | Desativada no modo tranquilo; opcional no clássico |
| Incubação | 3–5 minutos |
| Adulto | A partir de aproximadamente três dias de simulação |
| Sorteio | Família fixada na criação do ovo; cuidado determina ramificações |
| Comunicação | Offline como padrão; conexões opcionais no futuro |
| Economia | Ausente no núcleo; decoração pode ganhar moeda depois |
| Integração de firmware | Definir conforme launcher e projeto já existentes no Tab5 |

## 17. Fontes consultadas

[F1] Bandai — Manual GEN1 (reedição, documento de 2024):
https://www.bandai.com/amfile/file/download/file/3639/product/1309818/

[F2] Bandai — Manual oficial multilíngue Original:
https://tamagotchi-official.com/manual/toy/original/Original_webmanual.pdf

[F3] Bandai — What is Original Tamagotchi? Diferenças entre GEN1 e GEN2 e influência dos cuidados:
https://tamagotchi-official.com/us/series/original/news//01_166/

[F4] Bandai — FAQ Original: incubação, coleção, botões, disciplina e fim de vida:
https://tamagotchi-official.com/us/series/original/faq/

[F5] Bandai — Original GEN2, incluindo o jogo maior/menor:
https://www.bandai.com/original-tamagotchi-black-updated-packaging

[F6] M5Stack — Documentação de hardware do Tab5:
https://docs.m5stack.com/en/core/Tab5

As propostas de nomes, criaturas, regras, probabilidades, árvore de evolução, arquitetura e cronograma deste documento são concepção do projeto, não características atribuídas ao Tamagotchi original nem resultados já implementados.

