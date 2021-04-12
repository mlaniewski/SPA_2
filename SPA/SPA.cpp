// SPA.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#include "QueriesParser.h"
#include "Builder.h"
#include "SourceParser.h"
#include <string>
#include <iostream>
#include <list>

using namespace std;

int main(int argc, char* argv[])
{
	string variable, query = "", q1 = "", q2 = "", answer;
	string path(argv[1]);

	Builder builder;
	SourceParser sourceParser(builder);
	sourceParser.parse(path);
	shared_ptr<AST> ast = builder.getAST();

	//Lines below print tree to console. Uncomment for testing purposes.

	//tree<std::shared_ptr<Node>>::iterator s = ast.get()->astTree->begin();
	//tree<std::shared_ptr<Node>>::iterator e = ast.get()->astTree->end();
	//while (s != e) {
	//for (int i = 0; i < ast.get()->astTree->depth(s) - 2; ++i)
	//std::cout << " ";
	//std::cout << NODETYPE((*s)->nodeType) << std::endl;
	//++s;
	//}

	cout << "Ready\n";

	while (1)
	{
		getline(cin >> ws, q1);
		getline(cin >> ws, q2);
		query = q1 + " " + q2;

		if (query != "")
		{
			std::list<std::string> results;
			QueriesParser queriesParser(query, results, ast);
			queriesParser.parseQuery();

			if (queriesParser.results.empty()) {
				cout << "none";
			}
			else {
				for (auto iter = queriesParser.results.begin(); iter != queriesParser.results.end(); iter++) {
					if (iter != queriesParser.results.begin()) cout << ", ";
					cout << *iter;
				}
			}
			cout << "\n";
		}
	}


}

// Uruchomienie programu: Ctrl + F5 lub menu Debugowanie > Uruchom bez debugowania
// Debugowanie programu: F5 lub menu Debugowanie > Rozpocznij debugowanie

// Porady dotyczące rozpoczynania pracy:
//   1. Użyj okna Eksploratora rozwiązań, aby dodać pliki i zarządzać nimi
//   2. Użyj okna programu Team Explorer, aby nawiązać połączenie z kontrolą źródła
//   3. Użyj okna Dane wyjściowe, aby sprawdzić dane wyjściowe kompilacji i inne komunikaty
//   4. Użyj okna Lista błędów, aby zobaczyć błędy
//   5. Wybierz pozycję Projekt > Dodaj nowy element, aby utworzyć nowe pliki kodu, lub wybierz pozycję Projekt > Dodaj istniejący element, aby dodać istniejące pliku kodu do projektu
//   6. Aby w przyszłości ponownie otworzyć ten projekt, przejdź do pozycji Plik > Otwórz > Projekt i wybierz plik sln
