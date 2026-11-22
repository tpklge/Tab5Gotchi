# Requisitos e evidências

A base de escopo é tamagotchi_tab5_prompt_completo_codex.md; os dois documentos originais permanecem preservados. “Host” significa o mesmo núcleo compilado no computador. As evidências do Tab5 e seus limites estão em hardware.md.

| Requisito | Implementação | Evidência |
| --- | --- | --- |
| Seis famílias, 48 formas e seis ovos | Gerador, JSON por família e 54 bancos de poses | validate_assets.py; previews/familias.png e idosos-heranca.png |
| Identidade imutável e incubação | State e três RNGs separados | core_test.cpp; eclosão e recuperação no aparelho |
| Cuidados e oito indicadores | core/engine.cpp e telas de cuidado | Host: alimentação, limites, banheiro, banho, doença e sono |
| Dois adolescentes e três adultos por família | Tendências recentes, evolução e herança visual idosa | 36 caminhos no host; formas distintas verificadas pelo validador |
| Tranquilo, Contínuo e Clássico | tick/resume, limites e confirmação de Clássico | Host: ausência, relógio inválido/regressivo e fim de ciclo prolongado |
| Oito minijogos completos | Match persistido, regras e UI integradas | Host e regressão no Tab5; resultados em hardware.md |
| Recompensa única e cancelamento | Transação persistida e cooldown por jogo | Host; contador antes/depois no aparelho |
| Três perfis, nome e rotina | Settings/State por perfil | Host: isolamento; Tab5: troca e recuperação |
| Álbum, diário, decoração, retratos | Coleção, histórico e BMP no SD | UI compilada; capturas reais do Tab5 |
| Cenas, animação, tema retrô, A/B/C | LVGL e recursos RGB565+A8 | Dez cenários, 19 animações; validação e capturas |
| Música, efeitos e vozes | 48 WAVs, mixer de três canais | Formatos/loops no host e codec no Tab5; audição humana pendente |
| Persistência A/B e recuperação | CRC, temporário, flush e releitura | Host: truncamento/corrupção/backup; checkpoints no FAT real |
| SD obrigatório e erros sem perda de identidade | Pausa, mensagens e restauração confirmada | Verificações de código; ensaios destrutivos de energia pendentes |
| Atualização preservando dados | Instalador seletivo e manifesto | Atualizações reais com SHA-256/readback e saves preservados |
| Conteúdo editável e reproduzível | assets_src, gerador e validador | 179 arquivos imutáveis; cobertura em asset_coverage.md |
| Integração com launcher | Aplicativo no app1, mapa lido antes de gravar | Backup e comparação das regiões em hardware.md |
| Herança e visitas opcionais | extensions; flags desativadas no padrão | Build extras; 500 sementes e dois processos UDP no host |
| Distribuição, fontes e licenças | tools/package.py | Pacotes dist, manifestos e SHA256SUMS |

Cobertura visual estática não comprova evolução por 14 dias reais. Simulação acelerada usa o mesmo motor; o firmware normal não possui aceleração. Toques injetados exercitam LVGL e o controlador do jogo, sem substituir validação manual do sensor. Ensaios de corte de energia, autonomia, retenção do RTC e visita entre dois aparelhos continuam separados da validação funcional realizada.
