#ifndef ANALYZER_H
#define ANALYZER_H

#include <string>
#include <vector>
#include <unordered_map>




struct ZoneCount {

    std::string zone;

    int count;

};



struct SlotCount {

    std::string zone;

    int hour;
    int count;

};


class TripAnalyzer {

private:

    /* Hızlı erişim için Hash Map
       Zone ID ->Toplam Sürüş
     */
    std::unordered_map<std::string, int> map_bolgeSayaci;


    // ZoneID Saat ->Toplam Sürüş (Slotları birleşik anahtar olarak tutuyoruz)
    std::unordered_map<std::string, int> map_saatliBolge;



public:

    // Dosyadan okuma fonksiyonu:
    void ingestFile(const std::string& csvPath);


    // Raporlama fonksiyonları:

    std::vector<ZoneCount> topZones(int k = 10) const;

    std::vector<SlotCount> topBusySlots(int k = 10) const;

};

#endif
