# Formatos do pacote

`manifest.json`: schema_version=1, engine_api_version=1, content_version semântico, IDs de famílias, cenários e lista de caminhos relativos/tamanho/SHA-256. Caminhos absolutos e travessia são rejeitados. Conteúdo mutável em saves e album não faz parte do manifesto.

`pets/<id>/family.json`: índice, nome, oito formas e ovo (índice 8), arquivo de poses, dimensões, âncora, quadros, animações, quatro paletas e dois detalhes. Runtime atual: 96 × 96; banco de 32 poses. Para cada pose, 18432 bytes RGB565 little-endian seguidos de 9216 bytes alpha A8. Não é um arquivo LVGL com cabeçalho; o descritor é construído pelo carregador.

`scenes/*.rgb`: 640 × 360 RGB565 little-endian, sem cabeçalho. Escala visual para a área de cena. `ui/food*.rgb` e `decor*.rgb`: 64 × 64 RGB565+A8.

`ui/font.bin`: quatro uint32 LE (magic 0x31464742, quantidade, largura 36, altura 40); tabela (Unicode, avanço) com dois uint32 por glifo; em seguida células A8 de 36 × 40. Inclui Latin-1 e símbolos usados. Fonte original e licença em assets_src/fonts.

`data/foods.json`: id, name, kind (0 refeição, 1 fruta, 2 petisco), fullness, happiness, health e icon. `rules.json`: idades em milissegundos, incubação em segundos, desgaste por dez minutos, tolerância/limite offline. `games.json`: nomes e instruções; regras ficam no núcleo. `decor.json`: IDs, desenhos e condições gerais de desbloqueio.

Saves: cabeçalho de 16 bytes com magic 0x31475442, versão 1, tamanho do payload e CRC32 IEEE. Payload State inclui identidade, RNGs, atributos, perfil, rotina, evolução, transação, partida completa, cooldowns, coleção e diário circular de 40 eventos. Checkpoint a cada 60 segundos; ações críticas são persistidas imediatamente. Eventos não críticos podem perder até esse intervalo em desligamento abrupto.

Para modificar conteúdo, edite assets_src/generate.py, gere e valide. IDs existentes não podem ser reordenados em pacotes que convivem com saves. Mecânicas novas exigem firmware novo. Novas famílias compatíveis podem usar índices livres até o limite do catálogo e do álbum de 64 formas; nunca reaproveite um ID de save para outra criatura.

Perfis explicitamente apagados recebem marcador saves/emptyN. Um perfil sem save e com esse marcador continua vazio após reiniciar. Criar ovo remove o marcador e gera uma nova identidade; saves válidos têm prioridade sobre o marcador. Antes da exclusão confirmada, o save válido é arquivado em album/deleted-N-*.sav. Não remova marcadores durante atualizações do pacote.
