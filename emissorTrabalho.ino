/********************************
 * Projeto - Comunicação de Dados
 * -------------------- Emissor --------------------
 * Lê a string de entrada, remove espaços desnecessários, converte a string para binário usando a tabela ASCII 
 * e, em seguida, transmite a mensagem em binário usando o LED.
 * converterStringParaBinario: Esta função recebe uma string e retorna sua representação binária, convertendo cada caractere
 * para seu equivalente binário de 8 bits usando a tabela ASCII.
 * NRZ-L : Esta função pisca o LED de acordo com a codificação NRZ-L (Non-Return to Zero-Level). Um '0' representa
 * um LED aceso (HIGH) e '1' representa o LED apagado (LOW).
 * NRZ-I : Esta função pisca o LED de acordo com a codificação NRZ-I (Non-Return to Zero-Invert on ones). Um '0'
 * mantém o estado anterior do LED, enquanto um '1' inverte o estado do LED.
 * Alunos: Angélica, Mariana, Tiago, Mabylly
 **********************************/

#define PINO_LED 7
#define TEMPO_CICLO 1000

void setup() {
  Serial.begin(9600);
  pinMode(PINO_LED, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    String mensagem = Serial.readString();
    mensagem.trim();
    int tamanho_mensagem = mensagem.length();

    // método de transmissão
    Serial.println("Como deseja enviar os dados:");
    Serial.println("0 - NRZ_L");
    Serial.println("1 - NRZ_I");
    delay(5000); 

    int escolha_NR = Serial.parseInt();
    if (escolha_NR == 0) {
      Serial.println();
      Serial.println("Selecionado: NRZ_L");
    } else if (escolha_NR == 1) {
      Serial.println();
      Serial.println("Selecionado: NRZ_I");
    } else {
      Serial.println();
      Serial.println("Tipo não identificado");
    }

    Serial.println("Mensagem:");
    Serial.println(mensagem);
    Serial.println("Mensagem em binário: ");

    uint8_t *mensagem_binaria[tamanho_mensagem];

    for (uint8_t i = 0; i < tamanho_mensagem; i++) {
      uint8_t ascii = mensagem.charAt(i); // ASCII

      uint8_t *caractere_binario = (uint8_t *)malloc(sizeof(uint8_t) * 8);
      converterStringParaBinario(ascii, caractere_binario); // Transforma a letra para binário
      
      // Gera o checksum
      uint8_t *checksum = (uint8_t *)malloc(sizeof(uint8_t) * 3);
      criarCRC(caractere_binario, checksum);

      // palavra binaria com o crc adicionado
      uint8_t *binario = (uint8_t *)malloc(sizeof(uint8_t) * 11);
      uint8_t j = 0;
      for (uint8_t i = 0; i < 11; i++) {
        if (i >= 8) {
          binario[i] = checksum[j];
          j++;
        } else
          binario[i] = caractere_binario[i];

      } 

      for (int i = 0; i < 11; i++)
        Serial.print(binario[i]);
      Serial.println(" ");

      
      mensagem_binaria[i] = binario;
    } 


    Serial.println();
    Serial.println("Iniciando");

    switch (escolha_NR) {
    case 0:
      NRZ_L(tamanho_mensagem, mensagem_binaria);
      break;

    case 1:
      NRZ_I(tamanho_mensagem, mensagem_binaria);
      break;

    default:
      Serial.println("Erro ao enviar mensagem");
      break;
    }
    Serial.println("Fim");
  } 
  delay(10000); // tempo para nova palavra
} 

void converterStringParaBinario(uint8_t ascii, uint8_t *caractere_binario) {
  // Converte o código ascii para binario
  uint8_t i = 0;

  while (ascii > 0) {
    caractere_binario[i] = (uint8_t)(ascii % 2);
    ascii /= 2;
    i++;
  }

  // Preenche valores menores que 8
  do {
    caractere_binario[i] = 0;
    i++;
  } while (i < 8);


  for (uint8_t i = 0; i < 4; i++) {
    uint8_t aux = caractere_binario[i];
    caractere_binario[i] = caractere_binario[7 - i];
    caractere_binario[7 - i] = aux;
  }
}

void criarCRC(uint8_t *caractere_binario, uint8_t *checksum) {
  uint8_t codigo[] = {1, 0, 0, 1};
  uint8_t tamanho_codigo = sizeof(codigo) / sizeof(codigo[0]);

  // Os vazios são preenchidos com 0
  uint8_t aux[8 + tamanho_codigo - 1];
  uint8_t tamanho_aux = sizeof(aux) / sizeof(aux[0]);
  for (uint8_t i = 0; i < tamanho_aux; i++) {
    if (i >= 8)
      aux[i] = 0;
    else
      aux[i] = caractere_binario[i];
  }

  // Código CRC XOR
  for (uint8_t i = 0; i <= tamanho_aux - tamanho_codigo;) {
    for (uint8_t j = 0; j < tamanho_codigo; j++) {
      if (aux[i + j] == codigo[j]) {
        aux[i + j] = 0;
      } else {
        aux[i + j] = 1;
      }
    }

    for (i; i < tamanho_aux && aux[i] != 1; i++);
  }

  int j = 0;
  while (j < 3) {
    checksum[j] = aux[tamanho_aux - 3 + j];
    j++;
  }
}

void NRZ_I(int tamanho_mensagem, uint8_t *mensagem_binaria[]) {
  int8_t sinal;
  for (uint8_t i = 0; i < tamanho_mensagem; i++) {
    // Sincronizador
    for (int i = 0; i < 2; i++) {
      digitalWrite(PINO_LED, HIGH);
      delay(TEMPO_CICLO);
    }

    for (uint8_t j = 0; j < 11; j++) {
      // Quando vem um bit zero mantém o mesmo nível, com um inverte o nível
      if (j == 0) {
        sinal = mensagem_binaria[i][j] == 0 ? 1 : 0;
      } else {
        if (mensagem_binaria[i][j] == 1) {
          sinal = 1 - sinal; // Inverte o nível
        }
      }

      Serial.print(sinal);
      // Define o estado do LED com base no sinal atual
      if (sinal == 0) {
        digitalWrite(PINO_LED, LOW);  // Se o sinal for 0, define o LED como LOW (apagado).
      } else {
        digitalWrite(PINO_LED, HIGH); // Se o sinal for 1, define o LED como HIGH (aceso).
      }
      Serial.print(" ");
      delay(TEMPO_CICLO);
    }
    Serial.println();
  }

  digitalWrite(PINO_LED, LOW);
  delay(TEMPO_CICLO);
}

void NRZ_L(int tamanho_mensagem, uint8_t *mensagem_binaria[]) {
  for (uint8_t i = 0; i < tamanho_mensagem; i++) {
    // Sincronizador 
    for (int i = 0; i < 2; i++) {
      digitalWrite(PINO_LED, HIGH);
      delay(TEMPO_CICLO);
    }

    for (uint8_t j = 0; j < 11; j++) {
      if (mensagem_binaria[i][j] == 0) {
        Serial.print(1);
        digitalWrite(PINO_LED, HIGH);
      } else {
        Serial.print(0);
        digitalWrite(PINO_LED, LOW);
      }
      Serial.print(" ");
      delay(TEMPO_CICLO);
    }
    Serial.println();
  }

  digitalWrite(PINO_LED, LOW);
  delay(TEMPO_CICLO);
} 