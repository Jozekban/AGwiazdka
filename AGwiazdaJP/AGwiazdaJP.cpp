/*AUTHOR JOZEF PODLECKI*/

#include "stdafx.h"
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <queue>
#include <set>
#include <math.h>
#include <string.h>

using namespace std;

const int BRAK = -1;
const int START = 1;
const int KONIEC = 2;
const int KIERUNEK = 3;
const int PRZESZKODA = 5;

int szerokoscSiatki = 0;
int dlugoscSiatki = 0;
int szerokoscSiatkiIndex = 0;
int dlugoscSiatkiIndex = 0;
const int sciana = 5;
const int NiePrzejrzany = 0;
static int liczbaKierunkow = 8;
static int kierunekPrzeciwnyOffset = 4;
static int kierunki[8][2] = { 1, 0, 1, 1, 0, 1, -1, 1, -1, 0, -1, -1, 0, -1, 1, -1 };
int* siatka;
int* wierzcholkiPrzejrzane;
int* wierzcholkiNieOdwiedzone;
int* mapaKierunkow;

struct Punkt
{
	int x;
	int y;

	Punkt(){

	}

	Punkt(int x, int y){
		this->x = x;
		this->y = y;
	}
};

struct Wezel : Punkt
{
	int x;
	int y;
	double koszt; //suma wag krawêdzi
	double heurystyka;
	double f() const {
		return koszt + heurystyka;
	}

	Wezel(){

	}

	void zaaktualizujTrase(const Punkt& p){
		int xx = p.x - this->x;
		int yy = p.y - this->y;
		heurystyka = sqrt((double)(xx*xx + yy*yy));
	}
};

void ZaaktualizujWierzcholek(int *wierzcholki, int x, int y, int value){
	wierzcholki[x*szerokoscSiatki + y] = value;
}

int WartoscSiatkiWPunkcie(int x, int y){
	return siatka[x*szerokoscSiatki + y];
}

void UstawSiatkeWPunkcie(const Punkt p, int wartosc){
	siatka[p.y + p.x*szerokoscSiatki] = wartosc;
}

void UstawSiatkeWPunkcie(int x, int y, int wartosc){
	siatka[y + x*szerokoscSiatki] = wartosc;
}

bool PunktJestWwierzcholkuPrzejrzanym(int x, int y){
	return wierzcholkiPrzejrzane[x*szerokoscSiatki + y] == 1;
}

int WartoscPunktuWWierzcholku(int *wierzcholki, int x, int y){
	return wierzcholki[x*szerokoscSiatki + y];
}

/*
przeksztalca kierunek na przeciwny
np. kierunek (1,0) -> (-1:0)
bierze to sie z tad ze tablica 'kierunki' zostala zaprojektowana by
polozenie kierunku i kierunku przeciwnego w tablicy bylo oddzielone offsetem
*/
void ustawKierunek(int x, int y, int kierunekIndex){
	mapaKierunkow[x*szerokoscSiatki + y] = (kierunekIndex + kierunekPrzeciwnyOffset) % liczbaKierunkow;
}

int wezKierunek(int x, int y){
	return mapaKierunkow[x*szerokoscSiatki + y];
}

int przeciwnyKierunek(int kierunekIndex){
	return (kierunekIndex + kierunekPrzeciwnyOffset) % liczbaKierunkow;
}

static priority_queue<Wezel> kolejkaPriorytetowaWezlow;
static priority_queue<Wezel> kopiaKolejki;

bool operator<(const Wezel& a, const Wezel& b)
{
	return a.f() > b.f();
}

int main(){

	string nazwaPliku = "grid.txt";
	string sciezka = "";
	string opisTrasy = "";
	string pierwszaLinia;
	int rozmiarInt = sizeof(int);
	int sasiadX;
	int sasiadY;
	int wartosc;
	char znak;
	//Punkt punktStartowy(0, 1);
	//Punkt punktKoncowy(2, 7);
	Punkt punktStartowy(BRAK,BRAK);
	Punkt punktKoncowy(BRAK,BRAK);
	Wezel *wezel = new Wezel;
	Wezel *sasiadujacyWezel;

	cout << "A star algorithm -  JP" << endl;

	ifstream plik(nazwaPliku.c_str(), ios::binary | ios::ate);
	if (!plik.is_open()){
		cout << "nie znaleziono pliku grid.txt w biezacej lokalizacji, szukam parent..." << endl;
		
		plik.close();
		plik.clear();
		nazwaPliku = "..\\grid.txt";

		plik.open(nazwaPliku.c_str(), ios::binary | ios::ate);

		if (!plik.is_open()){
			cout << "nie znaleziono pliku grid.txt w parent" << endl;
			getchar();
			return -1;
		}
	}

	streamsize rozmiar = plik.tellg();
	plik.seekg(0, ios::beg);
	vector<char> bufor(rozmiar);
	plik.read(bufor.data(), rozmiar);
	plik.close();

	for (vector<char>::iterator it = bufor.begin(); *it != '\n'; ++it) {
		if (*it != ' ' && *it != '\r'){
			szerokoscSiatki++;
		}
	}

	for (vector<char>::iterator it = bufor.begin(); it != bufor.end(); ++it) {
		if (*it == '\r' || it + 1 == bufor.end()){
			dlugoscSiatki++;
		}
	}

	int iloscPol = szerokoscSiatki * dlugoscSiatki;

	szerokoscSiatkiIndex = szerokoscSiatki - 1;
	dlugoscSiatkiIndex = dlugoscSiatki - 1;

	siatka = new int[iloscPol];
	wierzcholkiPrzejrzane = new int[iloscPol];
	wierzcholkiNieOdwiedzone = new int[iloscPol];
	mapaKierunkow = new int[iloscPol];
	memset(wierzcholkiPrzejrzane, 0, iloscPol*rozmiarInt);
	memset(wierzcholkiNieOdwiedzone, 0, iloscPol*rozmiarInt);
	memset(mapaKierunkow, 0, iloscPol*rozmiarInt);
	memset(siatka, 0, iloscPol*rozmiarInt);
	int *ptr = siatka;
	int i = 0, j = 0, k = 0;

	for (vector<char>::iterator it = bufor.begin(); it != bufor.end(); ++it) {
		if (*it == '\r' || *it == '\n' || *it == ' ')
			continue;

		int liczba = *it - '0';

		if (liczba == START){
			punktStartowy.x = k / szerokoscSiatki;
			punktStartowy.y = k % szerokoscSiatki;
		}

		if (liczba == KONIEC){
			punktKoncowy.x = k / szerokoscSiatki;
			punktKoncowy.y = k % szerokoscSiatki;
		}

		siatka[k] = *it - '0';
		k++;
	}

	if (punktStartowy.x == BRAK){
		cout << "nie ustalono punktu startowego" << endl;
		getchar();
		return -1;
	}

	if (punktKoncowy.x == BRAK){
		cout << "nie ustalono punktu koncowego" << endl;
		getchar();
		return -1;
	}

	wezel->x = punktStartowy.x;
	wezel->y = punktStartowy.y;
	wezel->koszt = 0;
	wezel->zaaktualizujTrase(punktKoncowy);

	kolejkaPriorytetowaWezlow.push(*wezel);
	ZaaktualizujWierzcholek(wierzcholkiNieOdwiedzone, dlugoscSiatkiIndex, szerokoscSiatkiIndex, wezel->f());

	while (!kolejkaPriorytetowaWezlow.empty()){

		*wezel = kolejkaPriorytetowaWezlow.top();
		kolejkaPriorytetowaWezlow.pop();

		ZaaktualizujWierzcholek(wierzcholkiPrzejrzane, wezel->x, wezel->y, 1);
		ZaaktualizujWierzcholek(wierzcholkiNieOdwiedzone, dlugoscSiatkiIndex, szerokoscSiatkiIndex, 0);

		if (wezel->x == punktKoncowy.x && wezel->y == punktKoncowy.y){

			int x = punktKoncowy.x,
				y = punktKoncowy.y,
				kierunekIndex;

			while (!(x == punktStartowy.x && y == punktStartowy.y))
			{
				kierunekIndex = wezKierunek(x, y);
				znak = '0' + przeciwnyKierunek(kierunekIndex);
				sciezka = znak + sciezka;
				x += kierunki[kierunekIndex][0];
				y += kierunki[kierunekIndex][1];
			}

			while (!kolejkaPriorytetowaWezlow.empty()){
				kolejkaPriorytetowaWezlow.pop();
			}

			break;
		}

		for (int i = 0; i<liczbaKierunkow; i++){
			sasiadX = wezel->x + kierunki[i][0];
			sasiadY = wezel->y + kierunki[i][1];
			wartosc = WartoscSiatkiWPunkcie(sasiadX, sasiadY);

			if (sasiadX < 0 || sasiadX > dlugoscSiatkiIndex || sasiadY < 0 || sasiadY > szerokoscSiatkiIndex
				|| wartosc == sciana || PunktJestWwierzcholkuPrzejrzanym(sasiadX, sasiadY)){
				continue;
			}

			sasiadujacyWezel = new Wezel;
			sasiadujacyWezel->x = sasiadX;
			sasiadujacyWezel->y = sasiadY;
			sasiadujacyWezel->koszt = i % 2 == 0 ? 10 : 5;
			sasiadujacyWezel->zaaktualizujTrase(punktKoncowy);

			wartosc = WartoscPunktuWWierzcholku(wierzcholkiNieOdwiedzone, sasiadX, sasiadY);

			if (wartosc == NiePrzejrzany){
				ZaaktualizujWierzcholek(wierzcholkiNieOdwiedzone, dlugoscSiatkiIndex, szerokoscSiatkiIndex, sasiadujacyWezel->f());
				ustawKierunek(sasiadX, sasiadY, i);
				kolejkaPriorytetowaWezlow.push(*sasiadujacyWezel);
			}
			else if (wartosc > sasiadujacyWezel->f()){

				ZaaktualizujWierzcholek(wierzcholkiNieOdwiedzone, dlugoscSiatkiIndex, szerokoscSiatkiIndex, sasiadujacyWezel->f());
				ustawKierunek(sasiadX, sasiadY, i);
				Wezel wezelIterator = kolejkaPriorytetowaWezlow.top();

				while (!(wezelIterator.x == sasiadX && wezelIterator.y == sasiadY)){
					kopiaKolejki.push(wezelIterator);
					kolejkaPriorytetowaWezlow.pop();
					wezelIterator = kolejkaPriorytetowaWezlow.top();
				}
				kolejkaPriorytetowaWezlow.pop();

				if (kopiaKolejki.size() > kolejkaPriorytetowaWezlow.size()){
					swap(kopiaKolejki, kolejkaPriorytetowaWezlow);
				}

				while (!kopiaKolejki.empty()){
					kolejkaPriorytetowaWezlow.push(kopiaKolejki.top());
					kopiaKolejki.pop();
				}

				kolejkaPriorytetowaWezlow.push(*sasiadujacyWezel);
			}
			else delete sasiadujacyWezel;
		}
	}

	if (sciezka.length() > 0)
	{
		int kierunekIndex,
			x = punktStartowy.x,
			y = punktStartowy.y;

		for (int i = 0; i < sciezka.length(); i++)
		{
			znak = sciezka.at(i);
			kierunekIndex = atoi(&znak);
			x = x + kierunki[kierunekIndex][0];
			y = y + kierunki[kierunekIndex][1];
			/*
			opisTrasy = opisTrasy + "(" + to_string(x) + "," + to_string(y) +  ")";
			if (i + 1 < sciezka.length()){
				opisTrasy += " -> ";
			}
			*/
			UstawSiatkeWPunkcie(x, y, KIERUNEK);
		}
	}

	UstawSiatkeWPunkcie(punktStartowy, START);
	UstawSiatkeWPunkcie(punktKoncowy, KONIEC);
	/*
	cout << endl << endl;
	cout << "Punkt startowy : " << "(" + to_string(punktStartowy.x) + "," + to_string(punktStartowy.y) + ")" << endl;
	cout << "Punkt koncowy : " << "(" + to_string(punktKoncowy.x) + "," + to_string(punktKoncowy.y) + ")" << endl;
	cout << endl << endl;
	cout << "Trasa dotarcia : " << opisTrasy.c_str() << endl;
	cout << endl << endl;
	*/
	for (int i = 0; i<dlugoscSiatki; i++)
	{
		for (int j = 0; j<szerokoscSiatki; j++){
			switch (siatka[i*szerokoscSiatki + j]){
			case START:
				cout << "S  ";
				break;
			case PRZESZKODA:
				cout << "X  ";
				break;
			case KIERUNEK:
				cout << "+  ";
				break;
			case KONIEC:
				cout << "K  ";
				break;
			default:
				cout << "O  ";
			}
		}
		cout << endl << endl;
	}

	delete wezel;
	delete[] siatka;
	delete[] wierzcholkiPrzejrzane;
	delete[] wierzcholkiNieOdwiedzone;
	delete[] mapaKierunkow;
	getchar();
	return 0;
}
