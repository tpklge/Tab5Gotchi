# Bichinhos do Tab5

Jogo offline de bichinho virtual para M5Stack Tab5, com interface touch em português, seis famílias originais, 48 formas, seis ovos, cuidados, oito minijogos, três perfis e recursos no microSD.

Estado e evidências reais em [Docs/status.md](Docs/status.md). A documentação de concepção original está preservada em Docs. Teste host, compilação e teste no Tab5 são registrados separadamente.

## Downloads

Firmware, pacote do microSD, fontes e checksums estão nas [Releases do GitHub](https://github.com/tpklge/Tab5Gotchi/releases/latest). Use somente o aplicativo pelo launcher; preserve saves e álbum ao atualizar os recursos.

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

A navegação é por toque, sem controles A/B/C. Ajustes está organizado por categorias. Em Perfis, abra um dos três espaços, apague com confirmação ou crie um ovo em espaço vazio. A exclusão arquiva o ciclo antes de liberar o espaço. Nomes usam o teclado original encaixável do Tab5; Enter salva e Esc cancela. A detecção/digitação desse acessório ainda não foi confirmada no teste físico desta revisão.

## Licenças

Distribuição do projeto sob GPL-3.0 (LICENSE). Código, arte e áudio originais também estão disponíveis sob MIT (LICENSE-MIT); veja THIRD_PARTY_NOTICES.md. Bibliotecas e fonte conservam suas próprias licenças. Este é um projeto independente, sem recursos oficiais do Tamagotchi.
