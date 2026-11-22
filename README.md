# Bichinhos do Tab5

Jogo offline de bichinho virtual para M5Stack Tab5, com interface touch em português, seis famílias originais, 48 formas, seis ovos, cuidados, oito minijogos, três perfis e recursos no microSD.

Estado e evidências reais em [Docs/status.md](Docs/status.md). A documentação de concepção original está preservada em Docs. Teste host, compilação e teste no Tab5 são registrados separadamente.

## Instalar sem compilar

Baixe estes dois arquivos prontos:

- **[Baixar firmware .bin](https://github.com/tpklge/Tab5Gotchi/releases/latest/download/tab5gotchi-app.bin)** — aplicativo padrão para o launcher.
- **[Baixar recursos do microSD](https://github.com/tpklge/Tab5Gotchi/releases/latest/download/sdcard.zip)** — imagens, fontes e sons necessários ao jogo.

1. Extraia sdcard.zip e copie a pasta `tamagotchi` para a raiz do microSD. Preserve as pastas `tamagotchi/saves` e `tamagotchi/album` se já tiver progresso.
2. No launcher do Paludo, abra o gerenciador de aplicativos, selecione o arquivo `tab5gotchi-app.bin` e instale. Use o fluxo de atualização se o jogo já estiver instalado.
3. Abra o Tab5Gotchi pelo launcher. Não é necessário instalar ESP-IDF nem compilar o projeto.

O `.bin` contém somente o aplicativo e precisa dos recursos do SD. Não grave esse arquivo como uma imagem completa pelo endereço zero. Ele não inclui o launcher.

[Todos os arquivos e checksums](https://github.com/tpklge/Tab5Gotchi/releases/latest). A variante `extras` é opcional e sua integração entre dois aparelhos ainda não foi validada; para uso normal, escolha o firmware padrão acima.

## Compilar

Dependências do alvo fixadas em main/idf_component.yml e dependencies.lock: ESP-IDF 5.4.4, BSP Tab5 1.3.2, LVGL 9.3.0, port 2.9.0. Ative seu ESP-IDF e execute:

```sh
idf.py set-target esp32p4
idf.py build
```

Neste Mac, use o Python do ambiente IDF 5.4 antes de carregar export.sh; o Python global 3.14 não é o ambiente IDF instalado.

```sh
export PATH="$HOME/.espressif/python_env/idf5.4_py3.9_env/bin:$PATH"
. "$HOME/esp/esp-idf/export.sh"
idf.py build
```

**Com o launcher já instalado, não execute `idf.py flash`, não grave a tabela nem o bootloader do build.** O firmware de distribuição é somente o aplicativo. Instale o `.bin` pelo gerenciador do launcher. Para atualizar um slot que já contenha Tab5Gotchi, tools/flash_app.py lê e valida o mapa real, confere o descritor existente e grava somente aquele slot; o modo padrão é dry-run. Exemplo de atualização após conferir o inventário: `python tools/flash_app.py --port PORTA --slot SLOT --binary dist/tab5gotchi-app.bin`; acrescente `--apply` para executar o plano verificado.

## Gerar e verificar os recursos

O pacote pronto fica em sdcard/tamagotchi. Para reproduzir:

```sh
python3 -m venv .venv
.venv/bin/pip install -r requirements-tools.txt
.venv/bin/python tools/generate_assets.py
.venv/bin/python tools/validate_assets.py
tools/test_host.sh
tools/simulate.sh 123
```

A fonte editável de arte, poses, cenários e música está em assets_src/generate.py. Fonte TTF e licença estão incluídas. Não há downloads em tempo de execução.

## Instalar o cartão

Entre no modo disco USB-C pelo launcher. Identifique a unidade que contém a pasta launcher; copie tamagotchi à raiz. O instalador preserva saves e album e verifica os bytes copiados:

```sh
.venv/bin/python tools/install_assets.py '/Volumes/64 GB'
.venv/bin/python tools/install_assets.py '/Volumes/64 GB' --apply
```

O primeiro comando apenas descreve o destino. Ejete a unidade no sistema antes de sair do modo disco. Nunca formate o cartão. Não use sincronização com exclusão abrangente.

## Uso e recuperação

Consulte [guia de uso](Docs/user-guide.md), [arquitetura](Docs/architecture.md), [formatos](Docs/formats.md), [cobertura](Docs/asset_coverage.md) e [testes](Docs/test-plan.md).

Saves A/B em tamagotchi/saves; retratos, exportações e ciclos encerrados em tamagotchi/album. Não substitua essas pastas em atualizações. Erros de SD ou dois saves inválidos pausam o jogo, sem gerar outro ovo. A restauração de backup pede confirmação.

Com o jogo aberto, tools/device_test.py consulta estado, captura a tela e injeta entrada LVGL para regressões no hardware. Isso não substitui validação manual do sensor touch. Nenhum acelerador de crescimento é habilitado na compilação normal.

## Variante opcional e distribuição

`python tools/build_extras.py` compila herança local e visitas LAN em uma árvore isolada. A variante usa dependencies-extras.lock e precisa de um slot maior que o padrão; instale pelo gerenciador do launcher. Não foi validada entre dois Tab5. O firmware padrão não depende de Wi-Fi.

`python tools/package.py` prepara dist/tab5gotchi-app.bin, dist/tab5gotchi-extras-app.bin, dist/sdcard.zip, dist/source.zip e dist/tab5gotchi-release.zip, com manifest.json, SHA256SUMS e licenças. O pacote do SD exclui progresso e retratos. Copie a pasta inteira tamagotchi para fazer backup pessoal de saves, exportações, histórias e fotos.

Relatório do aparelho: [Docs/hardware.md](Docs/hardware.md). Matriz de requisitos: [Docs/requirements.md](Docs/requirements.md).

## Interface e perfis

A navegação é por toque, sem controles A/B/C. Ajustes está organizado por categorias. Em Perfis, abra um dos três espaços, apague com confirmação ou crie um ovo em espaço vazio. A exclusão arquiva o ciclo antes de liberar o espaço. Nomes usam o teclado original encaixável do Tab5; Enter salva e Esc cancela. O usuário confirmou funcionamento manual do teclado, do touch e reinícios aleatórios pelo botão. Os testes automatizados por USB complementam essa validação.

## Licenças

Distribuição do projeto sob GPL-3.0 (LICENSE). Código, arte e áudio originais também estão disponíveis sob MIT (LICENSE-MIT); veja THIRD_PARTY_NOTICES.md. Bibliotecas e fonte conservam suas próprias licenças. Este é um projeto independente, sem recursos oficiais do Tamagotchi.

## Versão 0.2.0

Animações guiadas pelos metadados, silhuetas, histórico consultável com aparência e parentesco, descobertas no quintal, contribuições de evolução limitadas por atividade e etapa, ganhos/cooldowns configuráveis, esconde-esconde com objetos e revelação, transições e dia/noite gradual. Bateria com porcentagem aproximada, brilho/volumes com `-`/`+` e apagamento configurável. Saves v1 são migrados para v2 com identidade preservada; mantenha o backup e use o firmware novo para abrir progresso atualizado.

Neste aparelho, a revisão atual usa app2 (slot 2); app1 conserva a versão anterior e app0 conserva Meshtastic. Para instalar uma revisão pelo launcher com backup e verificação, use `tools/deploy_launcher.py` com o jogo já no launcher. `--content` envia recursos alterados e o manifesto por último; `--boot` abre o slot instalado. O script exige a API local do launcher e Python com pyserial. Ele só substitui um slot explicitamente identificado como Tab5Gotchi e recusa um destino de instalação diferente do solicitado. Reorganização, quando necessária, é feita pelo próprio launcher.

`tools/smoke_device.py` verifica UI, migração, tela apagada e checkpoint, opção Nunca e minijogos no perfil corrente, restaurando o tempo de tela ao terminar. `tools/benchmark_device.py` mede uma partida com drenagem contínua da USB. `tools/test_device_resources.py` testa corrupção reversível de recursos e pacote incompatível, com cópia e restauração verificadas dos recursos; não modifica saves diretamente. Esses testes alteram o estado normal do jogo por suas ações. Logs/backups ficam em private/hardware e não entram na distribuição.
