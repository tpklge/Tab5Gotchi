# Verificação

Host: `tools/test_host.sh` compila o mesmo motor com ASan/UBSan e verifica identidade persistente, limites, recusa de alimento, banheiro e descarga, etapas do banho, saúde, sono, recuperação A/B, corrupção, separação de perfis, tempo tranquilo/contínuo, salto de relógio, 36 caminhos de evolução e os oito jogos. `tools/simulate.sh 123` acompanha 15 dias com relógio injetado. Nenhum acelerador é ativado no firmware normal.

Conteúdo: `python tools/validate_assets.py` verifica SHA-256/tamanhos, referências, poses não vazias, sequências válidas e diferentes, 54 formas/ovos, quatro paletas/dois detalhes, PCM, amplitude, duração e continuidade dos oito loops.

Hardware: registrar boot, revisão detectada pelo BSP, SD, perfil, identidade, sequência de save, heap e áudio. `tools/device_test.py` consulta a USB, injeta cliques no mesmo controlador ou eventos de ponteiro LVGL e obtém BMP da renderização física. Uma injeção LVGL não comprova o gesto humano no touch; registrar essa distinção. Capturas comprovam o framebuffer do Tab5, não são mockups do host.

Roteiro físico: incubar 3–5 min, sair ao launcher, reabrir e comparar ID/tempo; nascer; alimentar; banheiro e banho; dormir/acordar; jogar os oito jogos; trocar os três perfis; voltar ao primeiro; exportar/confirmar restauração; capturar álbum e cenário; repetir navegação e medir heap; testar ausência do pacote e recuperação sem substituir saves.

Som: verificar retorno do codec e blocos enviados sem erro; audição humana e qualidade acústica precisam ser declaradas separadamente. Bateria usa tensão, não autonomia estimada. Não testar corrupção/desligamento abrupto sobre progresso pessoal sem cópia isolada.
