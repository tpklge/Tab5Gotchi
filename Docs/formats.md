# Formatos do pacote

`manifest.json`: schema_version=1, engine_api_version=1, content_version semântico, IDs de famílias, cenários e lista de caminhos relativos/tamanho/SHA-256. Caminhos absolutos e travessia são rejeitados. Conteúdo mutável em saves e album não faz parte do manifesto.

`pets/<id>/family.json`: índice, nome, oito formas e ovo (índice 8), arquivo de poses, dimensões, âncora, quadros, animações, quatro paletas e dois detalhes. Runtime atual: 96 × 96; banco de 32 poses. Para cada pose, 18432 bytes RGB565 little-endian seguidos de 9216 bytes alpha A8. Não é um arquivo LVGL com cabeçalho; o descritor é construído pelo carregador.

`scenes/*.rgb`: 640 × 360 RGB565 little-endian, sem cabeçalho. Escala visual para a área de cena. `ui/food*.rgb` e `decor*.rgb`: 64 × 64 RGB565+A8.

`ui/font.bin`: quatro uint32 LE (magic 0x31464742, quantidade, largura 36, altura 40); tabela (Unicode, avanço) com dois uint32 por glifo; em seguida células A8 de 36 × 40. Inclui Latin-1 e símbolos usados. Fonte original e licença em assets_src/fonts.

`data/foods.json`: id, name, kind (0 refeição, 1 fruta, 2 petisco), fullness, happiness, health e icon. `rules.json`: idades em milissegundos, incubação em segundos, desgaste por dez minutos, tolerância/limite offline. `games.json`: nomes e instruções; regras ficam no núcleo. `decor.json`: IDs, desenhos e condições gerais de desbloqueio.

Saves: cabeçalho de 16 bytes com magic 0x31475442, versão 1, tamanho do payload e CRC32 IEEE. Payload State inclui identidade, RNGs, atributos, perfil, rotina, evolução, transação, partida completa, cooldowns, coleção e diário circular de 40 eventos. Checkpoint a cada 60 segundos; ações críticas são persistidas imediatamente. Eventos não críticos podem perder até esse intervalo em desligamento abrupto.

Ajustes: `saves/settingsa.cfg` e `settingsb.cfg` usam o mesmo cabeçalho/CRC, com payload Settings versão 2. O campo `screenTimeoutSeconds` é uint16: zero mantém a tela ligada; 30 a 1800, em múltiplos de 30, definem a inatividade antes de apagar a iluminação. O padrão é 120. Payloads versão 1 são migrados em memória preservando os controles e o perfil; brilho zero antigo passa a 10 para permitir o uso da tela. A próxima alteração salva a versão 2. Um arquivo com CRC válido mas valores inválidos também é rejeitado, tentando a cópia B. O formato dos saves dos bichinhos permanece igual.

Para modificar conteúdo, edite assets_src/generate.py, gere e valide. IDs existentes não podem ser reordenados em pacotes que convivem com saves. Mecânicas novas exigem firmware novo. Novas famílias compatíveis podem usar índices livres até o limite do catálogo e do álbum de 64 formas; nunca reaproveite um ID de save para outra criatura.

Perfis explicitamente apagados recebem marcador saves/emptyN. Um perfil sem save e com esse marcador continua vazio após reiniciar. Criar ovo remove o marcador e gera uma nova identidade; saves válidos têm prioridade sobre o marcador. Antes da exclusão confirmada, o save válido é arquivado em album/deleted-N-*.sav. Não remova marcadores durante atualizações do pacote.

## State v2 e conteúdo 0.2.0

O cabeçalho de arquivo permanece na versão 1 (magic/tamanho/CRC); a versão do payload State passa a 2. A estrutura v1 original tem 4872 bytes. O migrador só aceita o tamanho, CRC e formato exatos, copia o prefixo conhecido e inicializa a extensão. Não infere datas anteriores. Exportação, restauração e consulta a ciclos arquivados aceitam v1/v2. Não abra saves v2 com firmware antigo; backups anteriores ficam em private/hardware no computador e os binários substituídos são arquivados pelo launcher no SD.

Cada forma aceita frame_ms (40–2000, padrão 160), width/height até 192, frames até 128, anchor dentro da imagem, file do manifesto e animations como sequências de índices válidos. O total do banco fica limitado a 4 MiB. Os recursos entregues continuam 96×96, 32 poses. ui/hide0.rgb a hide5.rgb são RGB565+A8 128×96.

rules.json: action_cooldown_seconds, education_cooldown_seconds, reward_cooldown_seconds; action_duration_ms e explore_duration_ms; nap_duration_seconds; evolution_activity_cap; evolution_weights [tranquilo, explorador, social]; social_minimum_scale; game_reward [humor, bônus de vitória, vínculo, custo de energia]; action_gains com 20 linhas de três ganhos, em ordem Action::Meal a Action::Nap. A interpretação de cada coluna segue o tipo de ação (alimentação usa os dados de Food). O checkpoint_seconds passa a ser aplicado pelo firmware e aceita 10–3600 segundos.
