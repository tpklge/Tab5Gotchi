# Guia de uso

A primeira abertura cria um ovo surpresa. Sua família já fica salva, mesmo que o aplicativo seja fechado durante os três a cinco minutos de incubação. O modo tranquilo avança somente enquanto o jogo está aberto.

Toque no amigo para acariciar. Os botões da lateral levam a alimentação, água, higiene, sono, saúde, educação, jogos e exploração. Na higiene, use o vaso quando houver urgência e acione a descarga; limpe acidentes separadamente. O banho segue espuma → enxágue → secagem. Carinho ou Ninar ajuda a relaxar. O sono recupera energia. Soneca dura dez minutos de simulação ativa e acorda sozinha; Acordar permite interrompê-la. O tempo restante é salvo.

A ficha mostra oito indicadores; urgência do banheiro cresce quando ele precisa ir. Energia do pet e carga da bateria do Tab5 são coisas distintas. O cabeçalho mostra a carga estimada com `~` antes da porcentagem e `!` quando está em 15% ou menos. A leitura é atualizada a cada 15 segundos; `--` significa leitura indisponível. Em Ajustes > Tela e conforto também aparecem a tensão e o aviso de bateria baixa. A porcentagem usa uma curva aproximada para a bateria NP-F550 de duas células; carga, consumo e envelhecimento podem alterar a estimativa. Uma tensão isolada não confirma a presença da bateria nem se ela está carregando. Remédio é aceito somente durante indisposição. Repetir comida, elogio ou carinho imediatamente pode ser recusado.

Jokenpô: primeira pessoa a três vitórias. Par ou ímpar: números de 0 a 5, cinco rodadas e alternância de paridade. Esquerda/direita e maior/menor: cinco resultados válidos. Memória oferece 4, 6 ou 8 pares. Sequência musical também dá pistas visuais. Pega-frutinha usa arrasto e dura 30 segundos. Esconde-esconde tem três rodadas. Perder não causa doença.

Há três perfis independentes em Ajustes > Perfis. Um espaço vazio oferece Criar ovo; os demais oferecem Abrir e Apagar. Apagar pede confirmação e arquiva o ciclo no cartão antes de liberar o espaço. O espaço permanece vazio após reiniciar, até você escolher criar outro ovo. Exportar save grava um backup daquele perfil em album; restaurar pede confirmação. Encerrar ciclo na ficha arquiva o ciclo antes de criar outro ovo. Retrato grava BMP com o estado visível da tela. O álbum registra as descobertas, e o diário guarda os eventos recentes. Decorações surgem com convivência e brincadeiras.

Modo tranquilo: sem penalidade offline nem morte automática. Contínuo: crescimento na ausência, tolerância de duas horas e desgaste limitado. Clássico: permite fim de ciclo após mais de 48 horas consecutivas de necessidades críticas e saúde esgotada; precisa de confirmação. O limite vem de classic_risk_hours em data/rules.json. Retomada por relógio não mata automaticamente. Erro técnico nunca representa morte.

Ajustes reúne Som e volume, Tela e conforto, Perfis, Modo de cuidado, Backup e recuperação e acesso ao launcher. A navegação usa toque direto; os botões A/B/C foram retirados. Para mudar o nome, abra Ficha > Mudar nome e use o teclado original encaixável do Tab5. Enter salva, Esc cancela; há também botões de salvar e cancelar na tela.

Som e volume oferece botões de `-` e `+` para os três volumes (0–100%, em passos de 5%). Em Tela e conforto, ajuste o brilho entre 10 e 100%, também em passos de 5%. Os limites não dão a volta ao chegar ao mínimo ou máximo. Os ajustes são salvos no microSD e preservados após reiniciar.

Em Tela e conforto > Apagar em, use `-` e `+` para escolher de 30 segundos a 30 minutos sem uso, em passos de 30 segundos. O valor inicial é 2 minutos; reduza até Nunca para desativar. Ao terminar o tempo, a iluminação se apaga e as animações deixam de ser atualizadas; o jogo e os salvamentos continuam. Toque na tela ou pressione uma tecla do teclado encaixável para reacender no brilho escolhido. O primeiro toque ou tecla serve apenas para acordar a tela. Minijogos ativos mantêm a tela ligada, e a contagem recomeça ao finalizar a partida.

Mantenha o cartão inserido. Para atualizar, copie somente o conteúdo distribuído; não apague tamagotchi/saves nem tamagotchi/album. Em erro de recuperação, mantenha os arquivos inválidos para análise e restaure um backup conhecido.

## Coleção

Cada família possui bebê, criança, dois adolescentes, três adultos e idoso. O ovo sorteia a família uma única vez. Paleta, pequeno detalhe, personalidade e preferências dão identidade ao indivíduo.

| Família | Bebê | Adultos | Idoso |
| --- | --- | --- | --- |
| Capim | Capipim | Capimanso, Capitrilha, Capifesta | Capivô |
| Brotito | Sementim | Florim, Galhote, Frutino | Tronquim |
| Brasquito | Faisquito | Cristalume, Rocadraco, Faiscão | Brasancião |
| Gotim | Gotícula | Coralino, Marulhão, Bolhudo | Gotavô |
| Nuvito | Flocim | Pluminho, Tempestino, Arcoíro | Cirrô |
| Parafim | Porquinha | Serenóide, Voltix, Ritmô | Ferruvô |

Descanso e carinho, exploração e convivência em jogos alimentam tendências distintas. Os dois adolescentes podem chegar a qualquer adulto, conforme os cuidados recentes. O idoso conserva uma assinatura do adulto anterior. O álbum mostra descobertas e pistas; a folha previews/familias.png apresenta o elenco completo.

## Novidades da versão 0.2.0

O álbum mostra silhuetas das formas ainda não descobertas. Em Histórias, toque no indivíduo atual ou em um ciclo arquivado para ver aparência, datas disponíveis, marcos de crescimento, diário e pais/descendentes. Datas que não estavam nos saves antigos aparecem como não registradas. Os registros antigos continuam legíveis.

A exploração termina com uma descoberta do quintal; algumas descobertas liberam decoração. Variar as brincadeiras e manter bons cuidados contribui para a evolução. Cada atividade tem um limite por etapa, e carinho/educação repetidos têm retorno decrescente. Alimentação, higiene e tratamento continuam disponíveis para recuperar necessidades. O esconde-esconde mostra os seis objetos e revela o esconderijo ao resolver a rodada. As animações usam as sequências completas dos recursos e a luminosidade acompanha o horário gradualmente.

A versão atual migra saves para v2. Use o aplicativo novo no slot 2 deste aparelho; a cópia antiga no slot 1 não entende o formato v2. Não substitua os saves por backups sem escolher explicitamente uma restauração.

Na versão 0.2.1, Mudar nome informa se o teclado está conectado. Enter salva e Esc cancela. Ctrl+A limpa o campo; Backspace, Delete, setas, Home e End permitem editar. Maiúsculas e símbolos são reconhecidos. O texto em edição é mantido quando a interface precisa ser reconstruída.
