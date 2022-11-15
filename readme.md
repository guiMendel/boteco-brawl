# Instalacao em Mac

Para instalar o SDL e seus modulos em um mac, basta seguir os seugintes passos:

1. Instalar SDL2:

```bash
brew install sdl2
```

2. Instalar SDL2-image:

```bash
brew install sdl2_image
```

3. Instalar SDL2-mixer:

```bash
brew install sdl2_mixer
```

4. Se estiver usando vscode, inclua o diretorio de cabecalhos do SDL2 no seu projeto pelo arquivo .vscode.

Para tal, basta adicionar uma pasta chamada `.vscode` na raiz do projeto, e dentro dela um arquivo chamado `c_cpp_properties.json`.

Neste arquivo, inclua um objeto com uma chave `configurations`, que aponta para um array, e inclua neste array um outro objeto com a chave `includePath` apontando para um array tambem. Dentro deste array, insira a string com o caminho absoluto do diretorio (que voce pode encontrar executando o comando `sdl2-config --cflags`)

Eh possivel que seja necessario ter uma chave com a versao do arquivo tambem. Eu usei a versao 4.

Exemplo do conteudo do arquivo:

```json
{
  "configurations": [
    {
      "includePath": ["${workspaceFolder}/**", "/opt/homebrew/include/SDL2"]
    }
  ],
  "version": 4
}
```

> Fonte: https://code.visualstudio.com/docs/cpp/c-cpp-properties-schema-reference

# Configuracao do Makefile

O makefile deve incluir as seguintes flags no momento de montar o codigo executavel:

```bash
-lSDL2-2.0.0 -lSDL2_image -lSDL2_mixer
```

E recomenda-se que inclua tambem:

```bash
-std=c++17 -Wall -Wextra -pedantic
```

Para o bom funcionamento do SDL, eh necessario tambem incluir seus diretorios de libs e cabecalhos na compilacao. Felizmente, o SDL fornece uma ferramenta par aisso: o `sdl2-config`. Sendo assim, inclua no comando de compilacao do Makefile o seguinte:

```Makefile
$(shell sdl2-config --cflags) $(shell sdl2-config --libs)
```

Caso o helper falhe, é possível importar diretamente os arquivos de dentro da pasta do SDL na pasta do homebrew `Cellar`. Mas se fizer isso, sera necessario incluir os diretorios de libs e cabecalhos para o SDL2, SDl2-image e SDL2-mixer.

> Dica: vale rodar `brew info` para saber onde esta instalado o diretorio (exemplo: `brew info sdl2`)!

O `sdl2-config` ja fornece diretorios que contem os arquivos para o sdl e tambem para os outros modulos, como sdl-image.

Se voce executar o comando `sdl2-config --cflags`, vai ver onde estao incluidos os arquivos de cabecalho. Se nao tiver instalado o sdl2-image ou mixer, vai notar que os seus cabecalhos nao estarao presentes, mas quando instalar, vera eles serao incluidos neste diretorio.

=======

> Fontes do guia: https://medium.com/@edkins.sarah/set-up-sdl2-on-your-mac-without-xcode-6b0c33b723f7, https://www.reddit.com/r/sdl/comments/i5yx6f/cant_use_sdl_image_on_mac_without_xcode/

Vale ressaltar que a solucao aplicada neste projeto foi apenas inspirada na proposta pela fonte, e tem o objetivo de ser mais simples.
