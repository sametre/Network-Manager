# Ağ Kurulum Rehberi

## Önerilen Topoloji

```text
                 Modem / Router
                  192.168.1.1
                  /          \
                 /            \
       Windows Bilgisayar     POS Cihazı
        192.168.1.10          192.168.1.11
```

Her iki cihaz da aynı modem veya switch üzerinden aynı yerel ağa bağlı olmalıdır.

## Modem DHCP Rezervasyonu

Modem arayüzünde aşağıdaki isimlerden biriyle bulunan menüyü açın:

- DHCP Reservation
- Static Lease
- Address Reservation
- IP-MAC Binding

PC ve POS cihazının MAC adreslerini kendi IP adresleriyle eşleştirin.

## DHCP Havuzu

Statik cihaz adreslerini otomatik dağıtım aralığının dışında tutmak önerilir:

```text
Statik alan : 192.168.1.2 - 192.168.1.99
DHCP havuzu : 192.168.1.100 - 192.168.1.250
```

## Bağlantı Testi Başarısızsa

- POS cihazının açık ve ağa bağlı olduğunu kontrol edin.
- POS IP adresinin gerçekten `192.168.1.11` olduğunu doğrulayın.
- Bilgisayar ve POS cihazının alt ağ maskesini karşılaştırın.
- İki cihazın farklı misafir/VLAN ağlarında olmadığını kontrol edin.
- Modemde istemci izolasyonu veya AP isolation özelliğinin kapalı olduğundan emin olun.
- POS cihazı ICMP ping yanıtını kapatmış olabilir. Bu durumda asıl POS uygulamasının kullandığı TCP portuyla ayrıca test yapılmalıdır.
