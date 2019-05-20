#include <fstream>
#include <iostream>
#include <climits>
#include <math.h>

using namespace std;

#include <vector>
#include <string>

class Carlier {
    public:

        int ilosc_zadan; //dlugosc permutacji
        int Cmax_Schrage; //Cmax bez podzialu
        int Cmax_Scharage_podzial; //Cmax z podzialem
        std::vector<int*> permutacja;
        std::vector<int*> kolejka;
        std::vector<int*> oczekujace;
        std::vector<int*> optymalne;
        virtual ~Carlier() {
            for (int i = 0; i < ilosc_zadan; i++)
                delete permutacja[i];
        }
        //              p[0]       p[1] p[2] p[3]   p[4]
        // permutacja  |nr zadania | ri | pi | qi | czas zejscia z maszyny
        int wczytaj(std::string nazwa_pliku) {
            ifstream plik(nazwa_pliku.c_str());
            int * tmp;
            if (!plik.is_open()) {
                cout << "Blad otwarcia pliku!"<< endl;
                return 1;
            }
            permutacja.clear();
            plik >> ilosc_zadan;
            for (int i = 0; i < ilosc_zadan; i++) {
                tmp = new int[5];
                tmp[0] = i + 1;
                plik >> tmp[1] >> tmp[2] >> tmp[3];
                permutacja.push_back(tmp);
            }
            plik.close();
            Cmax_Schrage = 0;
            Cmax_Scharage_podzial = 0;
            return 0;
        }

        void shrage() {
            int t = 0;
            Cmax_Schrage = 0;
            sortuj();
            optymalne.clear();
            while (!kolejka.empty() || !oczekujace.empty()) {
                if (!kolejka.empty() && kolejka.back()[1] > t && oczekujace.empty())
                    t = kolejka.back()[1];
                while (!kolejka.empty() && kolejka.back()[1] <= t) {
                    dodaj_q(kolejka.back());
                    kolejka.pop_back();
                };
                optymalne.push_back(oczekujace.back());
                optymalne.back()[4] = optymalne.back()[2] + t;
                t += oczekujace.back()[2];
                Cmax_Schrage = max(Cmax_Schrage, t + oczekujace.back()[3]);
                oczekujace.pop_back();
            }
            permutacja = optymalne;
        }

        void shrage_podzial() {
            int t = 0;
            int * obecne;
            obecne = new int [4];
            obecne[3] = INT_MAX;
            obecne[1] = 0;
            obecne[2] = 0;
            sortuj();
            while (!kolejka.empty() || !oczekujace.empty()) {
                while (!kolejka.empty() && kolejka.back()[1] <= t) {
                    dodaj_q(kolejka.back());
                    if (kolejka.back()[3] > obecne[3]) { //jesli nowe ma wieksze q niz obecne
                        obecne[2] = t - kolejka.back()[1]; //
                        t = kolejka.back()[1];
                        if (obecne[2] > 0) {
                            dodaj_q(obecne);
                            obecne = new int [4];
                            obecne[3] = INT_MAX;
                            obecne[1] = 0;
                            obecne[2] = 0;
                        }
                    }
                    kolejka.pop_back();
                }
                if (oczekujace.empty()) {
                    t = kolejka.back()[1];
                } else {
                    for (int i = 0; i < 4; i++)
                        obecne[i] = oczekujace.back()[i];
                    t += oczekujace.back()[2];
                    Cmax_Scharage_podzial = max(Cmax_Scharage_podzial, t + oczekujace.back()[3]);
                    oczekujace.pop_back();
                }
            }
        }

        void sortuj_r() {
            quicksort(0, ilosc_zadan - 1);
        }

        void dodaj_q(int * dane) {
            vector<int*>::iterator it = oczekujace.begin();
            bool tu = false;
            while (it != oczekujace.end() && !tu) {
                if ((*it)[3] < dane[3])
                    it++;
                else {
                    tu = true;
                }
            }
            oczekujace.insert(it, dane);
        }

        // sortuje wedlug r i przypisuje do kolejki
        void sortuj() {
            kolejka.clear();
            for (int i = 0; i < ilosc_zadan; i++) {
                kolejka.push_back(permutacja[i]);
            }
            sortuj_r();
        }

        // wyznaczanie b
        int max_K(int Cmax) {
            int max_K = 0;
            bool nie_podzialnalazl = true;
            for (int i = ilosc_zadan-1; nie_podzialnalazl && i > 0; i--) {
                if (Cmax == (permutacja[i][4] + permutacja[i][3])) {
                    nie_podzialnalazl = false;
                    max_K = i;
                }
            }
            //cout<<"max_K: "<<max_K<<endl;
            return max_K;
        }
        // wyznaczanie a
        int min_K(int max_K, int Cmax) {
            int min_K, tmp = 0;
            for (min_K = 0; min_K < max_K; min_K++) {
                tmp = 0;
                for (int j = min_K; j <= max_K; j++) { //liczy ile wykonujan sie wszyskie zadania
                    tmp += permutacja[j][2];
                }
                if (Cmax == (tmp + permutacja[min_K][1] + permutacja[max_K][3])) {
                    return min_K;
                }
            }

            return min_K;
        }

        // wyznaczanie c
        int zadanie_interferencyjne_funkcja(int min_K, int max_K) {
            int tmp = -1;
            bool nie_podzialnalazl = true;
            for (int i = max_K; nie_podzialnalazl && i >= min_K; i--) {
            //pierwsze zad kotergo q jest mniejsze niz zadania b
                if (permutacja[i][3] < permutacja[max_K][3]) {
                    nie_podzialnalazl = false;
                    tmp = i;
                }
            }
            return tmp;
            //cout<<"min_K: "<<min_K<<endl;
        }

        int algorithm(int gorne_ogr) {
            int zadanie_c, poczatek_K, koniec_K, dolne_ogr = 0;
            int gorne_ogr_tmp = -1, r_pamiec, q_pamiec;
            int r_pi = INT_MAX,q_pi = INT_MAX, p_pi = 0;
            vector<int*> tmp;
            shrage();
            if (Cmax_Schrage < gorne_ogr) {
                gorne_ogr = Cmax_Schrage;
            }
            koniec_K = max_K(Cmax_Schrage);	//b
            poczatek_K = min_K(koniec_K, Cmax_Schrage); //a
            zadanie_c = zadanie_interferencyjne_funkcja(poczatek_K, koniec_K); // c
            //cout<<"a:"<<poczatek_K+1<<" b:"<<koniec_K+1<<" c:"<<zadanie_c+1<<endl;

            if (zadanie_c == -1) {//nie znalazl zadania c tzn Schrage optymalny
                return gorne_ogr;
            }
            for (int i = zadanie_c + 1; i <= koniec_K; i++) {
                r_pi = min(r_pi, permutacja[i][1]);
                p_pi += permutacja[i][2];
                q_pi = min(q_pi, permutacja[i][3]);
            }
            // zwiekszamy r
            r_pamiec = permutacja[zadanie_c][1];
            tmp = permutacja;
            permutacja[zadanie_c][1] = max(permutacja[zadanie_c][1], r_pi + p_pi);
            sortuj();
            shrage_podzial();
            dolne_ogr = Cmax_Scharage_podzial;
            if (dolne_ogr < gorne_ogr) {
                gorne_ogr_tmp = algorithm(gorne_ogr);
                // jezeli ok
                if (gorne_ogr_tmp != -1 && gorne_ogr_tmp < gorne_ogr)
                    gorne_ogr = gorne_ogr_tmp;
            }
            permutacja = tmp;
            permutacja[zadanie_c][1] = r_pamiec;

            // zwiekszamy q
            q_pamiec = permutacja[zadanie_c][3];
            permutacja[zadanie_c][3] = max(permutacja[zadanie_c][3],q_pi + p_pi);
            sortuj();
            shrage_podzial();
            dolne_ogr = Cmax_Scharage_podzial;
            if (dolne_ogr < gorne_ogr) {
                gorne_ogr_tmp = algorithm(gorne_ogr);
            // jezeli ok
                if (gorne_ogr_tmp != -1 && gorne_ogr_tmp < gorne_ogr)
                    gorne_ogr = gorne_ogr_tmp;
            }
            permutacja[zadanie_c][3] = q_pamiec;

            return gorne_ogr;
        }

        //Quick Sort
        void zamien(int pierwszy, int drugi) {
            int * tmp;
            tmp = kolejka[pierwszy];
            kolejka[pierwszy] = kolejka[drugi];
            kolejka[drugi] = tmp;
        }

        int podziel(int poczatek, int koniec, int pivot) {
            int wartosc_pivota = kolejka[pivot][1];
            int obecny = poczatek;
            zamien(pivot, koniec);
            for (int i = poczatek; i < koniec; i++) {
                if (kolejka[i][1] > wartosc_pivota) {

                    zamien(i, obecny);
                    obecny++;
                }
            }
            zamien(obecny, koniec);
            return obecny;
        }

        void quicksort(int lewy, int prawy) {
            if (prawy > lewy) {
                int nowy_pivot = podziel(lewy, prawy, (lewy + prawy) / 2);
                quicksort(lewy, nowy_pivot - 1);
                quicksort(nowy_pivot + 1, prawy);
            }
        }
};
int main(int argc, char** argv) {
    Carlier carlier;
    carlier.wczytaj("test.txt");
    cout << "Cmax : " << carlier.algorithm(INT_MAX) << endl;

    return 0;
}
