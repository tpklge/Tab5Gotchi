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
| Cenas, animação e tema retrô | LVGL e recursos RGB565+A8 | Dez cenários, sequências JSON no renderer, 19 animações; validação e capturas |
| Música, efeitos e vozes | 48 WAVs, mixer de três canais | Formatos/loops no host e codec no Tab5; audição humana pendente |
| Persistência A/B e recuperação | CRC, temporário, flush e releitura | Host: truncamento/corrupção/backup; checkpoints no FAT real |
| SD obrigatório e erros sem perda de identidade | Pausa, mensagens e restauração confirmada | Verificações de código; ensaios destrutivos de energia pendentes |
| Atualização preservando dados | Instalador seletivo e manifesto | Atualizações reais com SHA-256/readback e saves preservados |
| Conteúdo editável e reproduzível | assets_src, gerador e validador | 179 arquivos imutáveis; cobertura em asset_coverage.md |
| Integração com launcher | Revisão 0.2.0 no app2 pelo launcher; outros slots preservados | Backup e comparação das regiões em hardware.md |
| Herança e visitas opcionais | extensions; flags desativadas no padrão | Build extras; 500 sementes e dois processos UDP no host |
| Distribuição, fontes e licenças | tools/package.py | Pacotes dist, manifestos e SHA256SUMS |

Cobertura visual estática não comprova evolução por 14 dias reais. Simulação acelerada usa o mesmo motor; o firmware normal não possui aceleração. Toques injetados exercitam LVGL e o controlador do jogo, sem substituir validação manual do sensor. Ensaios de corte de energia, autonomia, retenção do RTC e visita entre dois aparelhos continuam separados da validação funcional realizada.

## Complementos da revisão 0.2.0

| Requisito | Resultado e evidência |
| --- | --- |
| Animação/metadados usados pelo firmware | FormAsset lê dimensões, âncora, quadros, período e sequências; validador exige cobertura; captura física |
| Evolução por convivência variada | Limite por atividade/etapa, contribuição da qualidade e bônus de tipo de jogo novo; testes de núcleo e progressão |
| Balanceamento em dados | Ganhos, cooldowns, durações, recompensas e checkpoint aplicados de rules.json |
| Exploração com eventos | Seis descobertas, eventRng independente e desbloqueio de decoração; teste determinístico |
| Álbum e histórias | Silhuetas, detalhe de indivíduos/ciclos, marcos, aparência e parentesco; migração e capturas |
| Esconde-esconde visual | Seis sprites de objetos, pet oculto durante busca e revelação na resolução |
| Integridade na execução | SHA-256 ao carregar recursos e antes de abrir áudio; erros identificados e fallback |
| Save antigo preservado | Layout v1 explícito e migração para v2; testes A/B, arquivo histórico e restauração |
| Bateria e inatividade | Estimativa identificada, limites de ajuste, sono da iluminação, despertar e Nunca; smoke no Tab5 |
| Meta de apresentação | Cerca de 33 fps nas janelas completas medidas em Pega-frutinha; não é garantia para toda combinação de cenas |

A/B/C, teclado touch para nomes e controles coloridos foram substituídos por decisões posteriores: navegação direta, teclado encaixável e controles cinza. Essas diferenças do prompt original foram mantidas nesta revisão. O usuário confirmou manualmente teclado, touch e alguns reinícios aleatórios pelo botão. Isso não substitui ensaio controlado de cortes de energia em cada fase de escrita.

Pacote incompatível também ensaiado no Tab5 nesta revisão: bloqueio de abertura, saves intactos por SHA-256 e recuperação após restaurar o manifesto.
