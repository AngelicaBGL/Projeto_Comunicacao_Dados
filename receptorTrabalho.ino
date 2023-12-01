/********************************
 * Projeto - Comunicação de Dados
 * -------------------- Receptor --------------------
 * Lê um sinal NRZ-L e NRZ-I de um sensor LDR. 
 * O sensor, por sua vez, lê a mensagem através do piscar do LED.
 * Alunos: Angélica, Mariana, Tiago, Mabylly
 **********************************/

#define PINO_LDR A0
#define LDR_THRESHOLD 100
#define PERIODO_REL_CICLO_MS 1000
#define TIPO_NR 0

void setup() {
  Serial.begin(9600);
  pinMode(PINO_LDR, INPUT);
}


void loop() {
  int8_t recebido = 0;
  int8_t comparar = 0;
  long int tempo1 = 0;
  long int tempo2 = 0;


  while (true) {
    tempo1 = millis();
    recebido = obterTensaoLDR();

    // Detectar sinal de sincronismo
    if (comparar == recebido && recebido == 1 && comparar != 0) {
      Serial.println("Recebendo a mensagem");
      tempo2 = millis();
      delay(PERIODO_REL_CICLO_MS - (tempo2 - tempo1));

      const int MAX_MENSAGENS = 64;
      int8_t **mensagem = (int8_t **)malloc(MAX_MENSAGENS * sizeof(int8_t *));
      
      int8_t *stream_mensagem = (int8_t*)malloc(sizeof(int8_t) * 11);
      obterStreamMensagem(stream_mensagem);
      mensagem[0] = stream_mensagem;
      Serial.println();
      int proxima_stream = 1;
      while (mensagemNaoCompleta(&tempo1)) {
      tempo2 = millis();
      delay(PERIODO_REL_CICLO_MS - (tempo2 - tempo1));

      int8_t *stream_mensagem = (int8_t*)malloc(sizeof(int8_t) * 11);
      obterStreamMensagem(stream_mensagem);
      mensagem[proxima_stream] = stream_mensagem;
      proxima_stream++;
      Serial.println();
      }

      Serial.println();
      Serial.println("Mensagem:");

      for (int i = 0; i < proxima_stream; i++) {
        uint8_t *bin_msg = (uint8_t*)malloc(sizeof(uint8_t) * 11);
        uint8_t *bin_char = (uint8_t *)malloc(sizeof(uint8_t) * 8);
        // tipo de decodificacao
        if (TIPO_NR == 0) {
          decodificarNRZ_L(mensagem[i], bin_msg);
        } else {
          decodificarNRZ_I(mensagem[i], bin_msg);
        }
        Serial.print(" ");
        printString(bin_msg, bin_char);

        free(bin_msg);
        free(bin_char);
        
      }

      Serial.println();

      for (int i = 0; i < proxima_stream; i++) {
        free(mensagem[i]);
      }

      free(mensagem);
    }

    comparar = recebido;
    delay(PERIODO_REL_CICLO_MS);
  }
}

bool mensagemNaoCompleta(long int *tempo1) {
  /*Verifica se o LED ainda está aceso por dois ciclos,
  para saber se a mensagem ainda continua*/

  int8_t recebido = 0;
  int8_t comparar = 0;

  for (uint8_t i = 0; i < 2; i++) {
    *tempo1 = millis();
    recebido = obterTensaoLDR();
    
    if (comparar == 1 && recebido == 1) {
      return true;
    }
    
    comparar = recebido;
    delay(PERIODO_REL_CICLO_MS);
  }
  
  return false;
}

void obterStreamMensagem(int8_t *stream_mensagem) {
  // Receber mensagem 
  for (int i = 0; i < 11; i++) {
    stream_mensagem[i] = obterTensaoLDR();
    Serial.print(stream_mensagem[i]);
    Serial.print(" ");
    delay(PERIODO_REL_CICLO_MS);
  }
}


int8_t obterTensaoLDR() {
  // Verifica a tensão do LDR. Se menor que o limite, retorna 0. Se não, retorna 1.
  return (uint16_t)analogRead(PINO_LDR) <= LDR_THRESHOLD ? 0 : 1;
}

void decodificarNRZ_L(int8_t *char_obtido, uint8_t *bin_msg) {
  // Decodificador NRZ-L, o decodificador inverte os valores de 0 e 1.
  for (uint8_t i = 0; i < 11; i++) {
    bin_msg[i] = char_obtido[i] == 0 ? 1 : 0;
  }
}

void decodificarNRZ_I(int8_t *char_obtido, uint8_t *bin_msg) {
  int ultimo;

  for (uint8_t i = 0; i < 11; i++) {
    // Para o primeiro caractere, o bit é invertido e o seu valor original é salvo na variável ultimo
    if (i != 0) {
      // Se o novo valor é igual o valor atual, 0, senão (inversão) é 1
      if (char_obtido[i] == ultimo) {
        bin_msg[i] = 0;
      } else {     
        bin_msg[i] = 1;
        ultimo = char_obtido[i];
      }
    } else {   
      if(char_obtido[i]==0){
        bin_msg[i]=1;
      }  else{
        bin_msg[i]=0;
      }
       bin_msg[i] = char_obtido[i]==0?1:0;
    }
    ultimo = char_obtido[i];
  }
}

bool verificarErro(uint8_t *bin_msg, uint8_t *bin_char) {
  uint8_t code[] = {1, 0, 0, 1};

  uint8_t aux[11];
  memcpy(aux, bin_msg, 11 * sizeof(uint8_t));

  // Utiliza o XOR entre os bits da mensagem e do CRC
  for (uint8_t i = 0; i <11 ; i++) {
    if (aux[i] == 1) {
      for (uint8_t j = 0; j < 4; j++) {
        aux[i + j] ^= code[j];
      }
    }
   // i += aux[i] == 0 ? 1 : tamanho_msg;
  }

  // Verifica se todos os bits do resultado são 0. Se não for, ocorreu erro na transmissao.
  for (uint8_t i = 0; i < 11; i++) {
    if (aux[i] == 1) {
      return false; 
    }
    if (i < 8) {
      bin_char[i] = bin_msg[i];
    }
  }

  return true;
}
int binToChar(uint8_t *binary) {
  int charValue = 0;

  for (int i = 0; i < 8; i++) {
    charValue += binary[i] * (1 << (7 - i));
  }

  return charValue;
}
void printString(uint8_t *bin_msg, uint8_t *bin_char) {
  if (verificarErro(bin_msg, bin_char)) {
    // Caso não tenha erro, converte para caractere ASCII e printa
    int charValue = binToChar(bin_char);
    Serial.print((char)charValue);
  } else {
    Serial.print("Erro na recepção da mensagem.");
  }
}

