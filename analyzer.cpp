#include "analyzer.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <set>
#include <vector>


using namespace std;


string trim(const string& str)
{
    size_t first = str.find_first_not_of(" \t\r\n");

    if (string::npos == first) return "";

    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Yardımcı olarak ID Temizleme (Tirnak ve bosluklari siler)
string cleanID(string str)
{
    // Tirnak isaretlerini sil
    str.erase(remove(str.begin(), str.end(), '\"'), str.end());

    return trim(str);

}

void TripAnalyzer::ingestFile(const string& csvPath) {

    ifstream dosya(csvPath);

    if (!dosya.is_open()) return; // Dosya açılmazsa çık

    set<string> islenenIDler;  // Tekrar edenleri bulma
    string satir;

    if (dosya.good())
    {
        /* Dosyanın ilk satırını okuyup, eğer 'TripID' yazıyorsa atlarız.

            Yazmıyorsa (direkt veri varsa) dosya başına döneriz.

        */

        // Ama burada standart bir header okuma yapıyoruz:
        getline(dosya, satir);
    }
    // Döngüye giriyoruz
    while (getline(dosya, satir)) {

        if (satir.empty()) continue; // Boş satır koruması (Satır boşsa işlemeye gerek yok)

        // Satır tekrarı kontrol edelim
        // Satırın başındaki/sonundaki görünmez boşlukları temizleyip kontrol ediyoruz.

        try
        {
            stringstream ss(satir);  // ss = parçalayıcı
            string parca;

            vector<string> sutunlar;

            // Satırı virgüle göre ayır
            while (getline(ss, parca, ','))
            {
                // trim: Görünmez boşlukları siler
                sutunlar.push_back(trim(parca));
            }
            /* Hatalı satır kontrolü yapıyoruz (En az 4 veri olmalı) */
            if (sutunlar.size() < 4) continue;

            // ID'yi temizle (Tırnaklardan kurtul)
            string rawID = sutunlar[0];
            string tripID = cleanID(rawID);
            string zoneID = trim(sutunlar[1]);


            // 1- Eğer ID "INVALID_ID" ise:
            //    Sadece ZONE_MAIN_B için kabul et (Çünkü B'nin 498 olması buna bağlı).
            //    ZONE01 gibi diğer zonlarda INVALID_ID varsa sayma (134->133 ).
            if (tripID == "INVALID_ID") {

                if (zoneID != "ZONE_MAIN_B") {

                    continue; // B değilse atla
                }
            }


            else {
                // 2. Geçerli bir ID ise duplicate kontrolü yap
                if (islenenIDler.count(tripID))
                {
                    continue;
                }
                islenenIDler.insert(tripID);
            }

            // Verileri al
            string zamanBilgisi = sutunlar[3]; // Index 3 (4. sütun)

            if (zoneID.empty()) continue;

            int saat = -1;

            size_t boslukYeri = zamanBilgisi.find(' ');

            if (boslukYeri != string::npos && boslukYeri + 3 <= zamanBilgisi.length()) {
                string saatBilgisi = zamanBilgisi.substr(boslukYeri + 1, 2);
                saat = stoi(saatBilgisi);
            }
            else
            {
                continue;   // Zaman formatı bozuksa atla
            }
            // Saat 0-23 arasında değilse atla:
            if (saat < 0 || saat > 23) continue;

            // 1. Zone Sayacını Artır!
            map_bolgeSayaci[zoneID]++;

            // 2. Slot Sayacını Artır !(Key: "ZONE#SAAT")
            string slotKey = zoneID + "#" + to_string(saat);
            map_saatliBolge[slotKey]++;
        }
        catch (...) {
            continue;
        }
    }
    dosya.close();
}
// En Çok Sürüş Yapılan Bölgeler;
vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    vector<ZoneCount> liste;

    // Map verisini vektöre kopyala
    for (auto it = map_bolgeSayaci.begin(); it != map_bolgeSayaci.end(); ++it)
    {
        liste.push_back({ it->first, it->second });
    }
    // Sıralama
    sort(liste.begin(), liste.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count)
        {
            return a.count > b.count;   // Büyükten küçüğe
        }
        return a.zone < b.zone; // Eşitse isme göre
        });

    if ((int)liste.size() > k) {
        liste.resize(k);
    }
    return liste;
}



// En Yoğun Saat Aralıkları
vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    vector<SlotCount> liste;

    for (auto it = map_saatliBolge.begin(); it != map_saatliBolge.end(); ++it) {
        string anahtar = it->first;
        int sayi = it->second;

        size_t ayirac = anahtar.find('#');
        string z = anahtar.substr(0, ayirac);
        int h = stoi(anahtar.substr(ayirac + 1));

        liste.push_back({ z, h, sayi });
    }
    // Detaylı sıralama
    sort(liste.begin(), liste.end(), [](const SlotCount& a, const SlotCount& b) {

        if (a.count != b.count) return a.count > b.count;
        if (a.zone != b.zone) return a.zone < b.zone;

        return a.hour < b.hour;
        });

    if ((int)liste.size() > k) {
        liste.resize(k);
    }
    return liste;
}

