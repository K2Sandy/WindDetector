#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

int16_t alamat_i2c_lcd = 0x27;
LiquidCrystal_I2C lcd(alamat_i2c_lcd, 16, 2);

// anemometer parameters
volatile byte rpmcount; // hitung signals
volatile unsigned long last_micros;
unsigned long timeold;
unsigned long timemeasure = 5.00; // detik
int timetoSleep = 1;               // menit
unsigned long sleepTime = 15;      // menit
unsigned long timeNow;
int countThing = 0;
int GPIO_pulse = 2;                // Arduino = D2
float rpm, rotasi_per_detik;       // rotasi/detik
float kecepatan_kilometer_per_jam; // kilometer/jam
float kecepatan_meter_per_detik;   //meter/detik
// arah angin
SoftwareSerial dataserial(7, 6); // D7,D6
String data, arah_angin, s_angin;
int a, b;

void setup()
{
  pinMode(10, OUTPUT);  //Led Tenang
  pinMode(11, OUTPUT); //Led Darurat
  pinMode(9, OUTPUT); //Alarm

  dataserial.begin(9600);
  pinMode(GPIO_pulse, INPUT_PULLUP);
  digitalWrite(GPIO_pulse, LOW);
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();
  detachInterrupt(digitalPinToInterrupt(GPIO_pulse));                         // memulai Interrupt pada nol
  attachInterrupt(digitalPinToInterrupt(GPIO_pulse), rpm_anemometer, RISING); //Inisialisasi pin interupt
  rpmcount = 0;
  rpm = 0;
  timeold = 0;
  timeNow = 0;
  lcd.setCursor(0, 0);
  lcd.print("Kec :");
  lcd.setCursor(0, 0);
  lcd.print("0.0 M/S  ");
  //lcd.clear();
} // end of setup

void loop()
{

  if ((millis() - timeold) >= timemeasure * 1000)
  {
    countThing++;
    detachInterrupt(digitalPinToInterrupt(GPIO_pulse));      // Menonaktifkan interrupt saat menghitung
    rotasi_per_detik = float(rpmcount) / float(timemeasure); // rotasi per detik
    //kecepatan_meter_per_detik = rotasi_per_detik; // rotasi/detik sebelum dikalibrasi untuk dijadikan meter per detik
    kecepatan_meter_per_detik = ((-0.0181 * (rotasi_per_detik * rotasi_per_detik)) + (1.3859 * rotasi_per_detik) + 1.4055); // meter/detik sesudah dikalibrasi dan sudah dijadikan meter per detik
    if (kecepatan_meter_per_detik <= 1.5)
    { // Minimum pembacaan sensor kecepatan angin adalah 1.5 meter/detik
      kecepatan_meter_per_detik = 0.0;
    }
    kecepatan_kilometer_per_jam = kecepatan_meter_per_detik * 3.6; // kilometer/jam
    if (kecepatan_kilometer_per_jam >= 4){
      digitalWrite(10, HIGH);  // turn the LED on (HIGH is the voltage level)
      delay(1000);                      // wait for a second
      digitalWrite(10, LOW);   // turn the LED off by making the voltage LOW
      delay(1000);   
      digitalWrite(9, HIGH);
      delay(5000);
       digitalWrite(9, HIGH);
      delay(5000);
    } else {
        digitalWrite(11, HIGH);
        delay(1000);
        digitalWrite(11, LOW);
        delay(1000);
        digitalWrite(9, LOW);
        delay(1000);

    }
    Serial.print("rotasi_per_detik=");
    Serial.print(rotasi_per_detik);
    Serial.print("   kecepatan_meter_per_detik="); // Minimal kecepatan angin yang dapat dibaca sensor adalah
    Serial.print(kecepatan_meter_per_detik);       // 1.5 meter/detik dan maksimum 30 meter/detik.
    Serial.print("   kecepatan_kilometer_per_jam=");
    Serial.print(kecepatan_kilometer_per_jam);
    Serial.println("   ");

     
    
    lcd.setCursor(0, 0);
    lcd.print("Kec :");
    lcd.setCursor(0, 0);
    lcd.print(kecepatan_meter_per_detik);
    lcd.print(" M/S  ");
    if (countThing == 1) // kirim data per 10 detik sekali
    {
      Serial.println("Mengirim data ke server");
      countThing = 0;
    }
    timeold = millis();
    rpmcount = 0;
    attachInterrupt(digitalPinToInterrupt(GPIO_pulse), rpm_anemometer, RISING); // enable interrupt
  } 
  
  if (dataserial.available()) // Jika ada data yang diterima dari sensor
  {
    data = dataserial.readString(); // data yang diterima dari sensor berawalan tanda * dan diakhiri tanda #, contoh *1#
    a = data.indexOf("*"); // a adalah index tanda *
    b = data.indexOf("#"); // b adalah index tanda #
    s_angin = data.substring(a + 1, b); // membuang tanda * dan # sehingga di dapat nilai dari arah angin
    if (s_angin.equals("1")) { // jika nilai dari sensor 1 maka arah angin utara
      arah_angin = "utara     ";
    }
    if (s_angin.equals("2")) {
      arah_angin = "timur laut";
    }
    if (s_angin.equals("3")) {
      arah_angin = "timur     ";
    }
    if (s_angin.equals("4")) {
      arah_angin = "tenggara  ";
    }
    if (s_angin.equals("5")) {
      arah_angin = "selatan   ";
    }
    if (s_angin.equals("6")) {
      arah_angin = "barat daya";
    }
    if (s_angin.equals("7")) {
      arah_angin = "barat     ";
    }
    if (s_angin.equals("8")) {
      arah_angin = "barat laut";
    }
    Serial.println(arah_angin);
    lcd.setCursor(0, 1); // menampilkan pada lcd kolom 0, baris 0
    lcd.print(arah_angin);
  }
  

} // end of loop

/********************************************************************************
* // Fungsi ini dipanggil setiap kali magnet/interrupt terdeteksi oleh MCU
*********************************************************************************/
// Fungsi void rpm_anemometer() untuk menghitung putaran per menit (RPM)
void rpm_anemometer()
{
  // Cek apakah waktu sejak deteksi terakhir sudah lebih dari atau sama dengan 5000 mikrodetik (5 milidetik)
  if (long(micros() - last_micros) >= 5000)
  {
    // Jika ya, maka tambahkan satu ke penghitung rpm (rpmcount)
    rpmcount++;
    // Perbarui waktu terakhir ketika magnet terdeteksi dengan waktu saat ini
    last_micros = micros();
  }
  // Catatan: Kode untuk mencetak ke serial dihilangkan, biasanya digunakan untuk debugging
  // Serial.println("***** magnet terdeteksi *****");
}
