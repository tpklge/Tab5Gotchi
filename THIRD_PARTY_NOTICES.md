# Procedência e licenças

O motor, interface, geradores de pixel art e composições WAV deste projeto são originais, sob MIT (LICENSE-MIT). Não há recursos oficiais do Tamagotchi. As formas são desenhos por família/estágio produzidos offline; não há IA no aparelho.

- ESP-IDF 5.4.4: Apache-2.0, Espressif. SDK externo em `~/esp/esp-idf`.
- BSP espressif/m5stack_tab5 1.3.2 e esp_lvgl_port 2.9.0: Apache-2.0. Dependências transitivas e licenças ficam preservadas em managed_components e dependencies.lock.
- LVGL 9.3.0: MIT. O jogo usa APIs públicas e fonte de diagnóstico da biblioteca.
- DejaVu Sans: licença própria permissiva preservada em assets_src/fonts/LICENSE-DejaVu.txt; fonte bitmap derivada fica no SD. Arquivo TTF proveniente da distribuição LVGL 9.3.0, scripts/built_in_font.
- Pillow, NumPy, pyserial, pyusb e clang-format são ferramentas do host; não são incorporadas ao firmware.

O launcher Tab5Launcher do Paludo (GPL-3.0) foi inspecionado e compilado para confirmar o ambiente. Seu protocolo público de manutenção e sua WebGUI são usados na instalação. Não incorporamos seu código ao firmware. As APIs/pinagens do BSP oficial são usadas diretamente; não copiamos código de jogos vizinhos.

Referências técnicas consultadas: https://docs.m5stack.com/en/core/Tab5 e https://docs.espressif.com/projects/esp-idf/en/v5.4.4/esp32p4/api-reference/system/ota.html .

A distribuição neste repositório preserva a licença GPL-3.0 originalmente escolhida pelo proprietário (LICENSE). A licença MIT dos componentes originais permanece disponível em LICENSE-MIT.
