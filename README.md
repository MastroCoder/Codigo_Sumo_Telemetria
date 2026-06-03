# SISTEMA DE TELEMETRIA PARA A AVALIAÇÃO DE DESEMPENHO DE UM ROBÔ DE MEGA SUMÔ AUTÔNOMO

Este repositório armazena o código dedicado ao controle de um robô de sumô autônomo. Este sistema também é responsável pela obtenção de medições para o registro em arquivos CSV internos que, após o fim de uma partida, são enviados para uma API externa para a criação de uma visualização.

## Dependências externas

O projeto tem como dependências externas principais três bibliotecas. São elas:

- [Itamotorino](https://github.com/facens-omegabotz/Itamotorino-esp32) - Inserida como submódulo git;
- [IRremote](https://github.com/Arduino-IRremote/Arduino-IRremote) - Inserida como dependência em `platformio.ini`;
- [qtr-sensors-arduino](github.com/pololu/qtr-sensors-arduino.git#4.0.0) - Inserida como dependência em `platformio.ini`.

## Sistema de arquivos

O projeto usa o sistema de arquivos [LittleFS](https://github.com/littlefs-project/littlefs), implementado a partir das abstrações fornecidas pela Espressif. O uso do sistema de arquivos é dependente da aplicação de uma tabela de partição, como a representada em `partition.csv`. Dado que o projeto utiliza-se da framework de Arduino para facilitar o uso de dependencias como IRremote, não é possível definir diretamente o tipo de armazenamento como `littlefs` em `partition.csv`, mas o funcionamento não é alterado.

```csv:partition.csv
# Name,     Type, SubType,  Offset,  Size,    Flags
nvs,        data, nvs,      0x9000,  0x5000,
otadata,    data, ota,      0xe000,  0x2000,
app0,       app,  ota_0,    0x10000, 0x140000,
app1,       app,  ota_1,    0x150000,0x140000,
storage,    data, spiffs,   0x290000,0x160000,
coredump,   data, coredump, 0x3F0000,0x10000,
```

### Passos para uso do sistema de arquivos

O particionamento do microcontrolador deve ser realizado a partir do PlatformIO. No menu principal, seguir o caminho:

`Project tasks > esp32doit-devkit-v1 > Platform > Build Filesystem Image`

Após o processo, com o ESP32 conectado ao computador, seguir o caminho:

`Project tasks > esp32doit-devkit-v1 > Platform > Upload Filesystem Image`

Após estes passos, fica habilitado o uso normal de LittleFS.