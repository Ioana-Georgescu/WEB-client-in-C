# WEB-client-in-C
Client web in C pentru un server; cereri HTTP, interactiune cu REST API si JSON

Conectare server:
HOST: ec2-3-8-116-10.eu-west-2.compute.amazonaws.com, PORT: 8080

Functionalitati server:

Inregistrarea unui cont
• Ruta de acces: POST /api/v1/tema/auth/register

Autentificare
• Ruta de acces: POST /api/v1/tema/auth/login

Cerere de acces in biblioteca
• Ruta de acces: GET /api/v1/tema/library/access

Vizualizarea informatiilor sumare despre toate cartile
• Ruta de acces: GET /api/v1/tema/library/books

Vizualizarea detaliilor despre o carte
• Ruta de acces: GET /api/v1/tema/library/books/:bookId

Adaugarea unei carti
• Ruta de acces: POST /api/v1/tema/library/books

Stergerea unei carti
• Ruta de acces: DELETE /api/v1/tema/library/books/:bookId

Logout
• Ruta de acces: GET /api/v1/tema/auth/logout


Clientul trimite comenzi de la tastatura:
• register
• login
• enter library - cere acces in biblioteca
• get books - cere toate cartile de pe server
• get book - cere informatie despre o carte
• add book - adauga o carte
• delete book - sterge o carte
• logout
• exit - se inchide programul


Fisierele incluse:
client.c: functia main
requests.c: cate o functie pentru fiecare tip de comanda ce poate fi primit
	de la utilizator
aux.c: functii auxiliare
aux.h: fisierul cu toate define-urile si declaratiile functiilor auxiliare
parson.h si parson.c: bilbliotaca pentru json
Makefile


client.c:
main:
	Intr-un while(1) citeste o linie de la tastatura
	Daca o recunoaste ca o comanda valida apeleaza functia corespunzatoare sau
		iese din program in cazul lui exit
	O comanda valida se considera oricare dintre comenzile de la punctul 5 din
		cerinta
	Sirul de caractere command in care se citeste comanda primita de la
		tastatura are lungimea maxima de 15 deoarece lungimea maxima a unei
		comenzi valide este 13 si am ales sa nu consider comanda valida daca
		este urmata de spatiu sau alte caractere si vreau sa pot verivica si
		aceste casiri cu strcmp
	In cookie si token_JWT se vor salva cookie-urile si tokenul pentru sesiunea
		curenta
	Daca se cere efectuarea unei comenzi care cere demonstrarea unei logari sau
		autentificari (va trebui sa trimita ori cookie-ul ori token-ul la
		server) si acestea lipsesc (== NULL) se va afisa un mesaj de eroare
		corespunzator fara sa se trimita mesaj la server pentru o functionare
		mai eficienta.
	Aplicatia permite reintrarea in biblioteca si cand exista deja un token jwt
		valid neexpirat si continua sa foloseasca cel mai recent obtinut token.
	Aplicatia permite oricand inregistrarea de noi useri.
	Sirul de caractere username tine minte userul logat sau "\0" daca nu este
		logat niciun user. Aplicatia nu permite logarea cat timp un user este
		logat deja.

aux.h:
	dimensiunea unui buffer pentru trimiterea/primirea de mesaje catre/de la
		server este cea din laborator (BUFLEN = 4096). La fel si pentru
		LINELEN, utilizata ca dimensiunea maxima a unei linii dintr-un mesaj
	am facut define-uri pentru:
		sirul "\0"
		sirurile de terminatori dintr-un mesaj http
		prima parte a primei linii a unui mesaj http pentru a putea ajunge
			mai usor la comentariul adaugat in raspunsurile serverului
		ip-ul si portul hostului, pentru o lizibilitate mai mare a codului
			pentru conectare
		url-urile pentru fiecare pagina cu care se lucreaza (pagina de login,
			logout, biblioteca, etc)
		headerele Host, Content-Type si Accept deoarece sunt identice in toate
			cererile ce vor fi trimise in cadrul temei
	am inclus si macro-ul DIE pe care il folosesc pentru a trata erorile ce
		apar la conexiunea TCP cu serverul

aux.c:
	open_connection: deschide conexiunea cu serverul si intoarece socketul

	get_response: trimite un mesaj pe socket la server si returneaza raspunsul
		pentru a evita situatia in care conexiunea se inchide intre trimterea
		mesajului si primirea raspunsului, la inceputulfunctiei se deschide
		conexiunea, apoi se trimite mesajul, se primeste raspunsul si se
		inchide conexiunea
		am pastrat si verificarea inchiderii conexiunii de catre server inainte
		de a se trimite/primi mesajul (0 biti trimisi/primiti la prima incercare
		de trimitere/primire), caz in care mai apelez o data open_connection,
		dar nu ar trebui sa se ajunga la acest caz

	add: conacteneaza o linie la mesaj si adauga terminatorul delinie pentru un
		mesaj http

	get: creaza o cerere get
		primeste url-ul la care se face cererea, parametrii (id-ul cartii),
		cookie-urile salvate ca u singur string, si token-ul jwt
		nu are parametru pentru query, intrucat in cadrul temei nu se realizeaza
		queriuri
		adauga la mesaj numai headerele pe care le primeste efectiv, deoarece
		poate fi apelat fara token, cookie sau parametru. Nu verifica si url-ul
		deoarece e un camp care nu lipseste niciodata

	post: creaza o cerere post
		ca si get, doar ca in loc de parametrii (cererile post se fac numai la
		url-uri care au fost deja definite in aux.h), are un payload. Payload-ul
		e formatat cu parson, deci pentru a calcula Content-Length e de ajuns
		sa se apeleze strlen

	del: creaza o cerere delete
		functioneaza ca si get

	check_code: primeste raspunsul serverului si il analizeaza
		intoarce codul mesajului in parametrul code
		intoarce corpul mesajului (json / array de json / mesaj plain text) sau
		NULL daca nu exista; util pentru comenzile care se asteapta sa
		primeasca informatii de la server (get_books, get_book)
		mesajului i se face o copie deoarece va fi prelucrat cu strtok si va fi
		nevoie de el in forma originala daca s-a primit 429
		se salveaza in body corpul mesajului (tor ce este dupa headere si linia
		goala)
		se salveaza in line prima linie a mesajului
		se afiseaza comentariul de dupa cod
		se extrage codul cu strtok si se converteste in intreg
		se verifica codul
		daca este 429 se parcurge copia cu strtok pana la header-ul Retry-After
		si se afiseaza numarul de secunde ce trebuie asteptate
		daca e orice alt cod de eroare se verifica formatul lui body:
			daca contine [{" e array de json; e parsat folosind functiile din
			biblioteca parson si se afiseaza mesajul din campul error al
			fiecarui element
			daca contine {" e json; se parseasa si se afiseaza mesajul din
			campul error
			daca nu, e plain text si se afiseaza ca atare
		se afiseaza un "\n" pentru formatarea mai frumoasa a raspunsului afisat
		la consola

	timeout: verfica daca a expirat cookie sau token
		cookie-urile primite la tema au durata de viata de 15552000 secunde
		timeout primeste codul extras dintr-un raspuns si daca este unul dintre
		codurile ce marcheaza un raspuns asociat cu expirarea unor credentiale
		"uita" cookie-ul, token-ul si username-ul pentru a orta clientul sa se
		logheze din nou
		codurile considerate:requests.c: fiecare functie are numele comnzii pe care 
			401 Unauthorized
			403 Forbidden
			419 Page Expired
			440 Login Time-out
			498 Invalid Token


requests.c: fiecare functie are nume similar cu al comenzii pe care o
		implementeaza

	register_client: am facut teste si am observat ca serverul nu pare sa aiba
		o limita pentru lungimea username-ului si a parolei si permite spatii
		in ele. Am decis sa impun eu o limita de 50 de caractere pentru
		username si parola
		citeste username si password de la tastatura si construieste peyload-ul
		cu ajutorul functiilor din parson face un mesaj post pe care il trimite
		cu get_response si verifica raspunsul cu check_code
		nu se asteapta la un raspnus, deci nu tine minte sirul de caractere
		intors de check_code
		nu trebuie sa dovedeasca vreo logare sau autentificare deci nu apeleaza
		timeout

	login_client: returneaza cookie sau NULL la cereri neacceptate
		primeste username ca parametru pentru a citi in el valoarea
		primita de la tastatura
		register_client doar creaza conturi si nu influenteaza cine e userul
		curent; userul curent e stabilit de login
		se citesc username si parola, se parseaza si se trimit ca la
		register_client
		daca nu s-a primit eroare (cod < 300) atunci se extrage cookie din
		header-ul Set-Cookie si se reurneaza pentru a fi tinut minte de
		variabila din main
		daca s-a primit o eroare => cedentialele introduse nu sunt valide si nu
		trebuie tinut minte username-ul => se sterge copiindu-se "\0" peste el

	lib_acc: returneaza token sau NULL la cereri neacceptate
		trimite o cerere get si daca nu s-a primit eroare foloseste functiile
		din parson pentru a extrage token-ul din campul token al mesajlui
		intors de check_code; va trebui intai parsat din string in obiect json,
		apoi se poate extrage vaaloarea
		daca s-a primit eroare se apeleaza timeout pentru ca a cererea get a
		continut un cookie care e posibil sa fi expirat

	get_books:
		trimite o cerere get; e nevoie doar de token nu si de cookie, dar eu
		le-am inclus pe amandoua in mesaj
		trebuia sa primesc un singur json asa ca daca n-am cod de eroare si
		gasesc intr-adevar un json in stringul intors de check_code (contine
		{"), il parsez si extrag id-ul si titlu s le afisez
		daca n-am primit eroare si nu exista un json in mesaj inseamna ca
		biblioteca e goala
		in caz de eroare apelez timeout pentru ca am inclus credentiale in
		cererea http

	get_book:
		citeste un rand de la tastatura si cat timp nu e un id valid (un sir de
		cifre) cere reintroducerea lui
		id-ul a fost citit ca string deci poate fi trimis asa cum este pe
		pozitia de parametru a lui get
		mesajul intors de check_code este parsat si sunt extrase informatiile
		despre carte
		in caz de eroare se apeleaza timeout

	add_book:
		cere introducerea datelor dela tastatura; procedeaza cu verificarea
		numarului de pagini ca get_book cu id
		construieste un obiect json cu dateleprimite si il trimite ca payload
		functiei post
		cum nu e nevoie si de cookie pentru operatiile care cer demonstrarea
		accesului la biblioteca, functia post nu are si parametru de cookie
		get avea deoarece am folosit aceeasi functie si pentru cereri care
		cereau demonstrarea logarii si pentru cereri care cereau token-ul
		pentru biblioteca
		apeleaza timeout in caz de eroare

	del_book: delete_book
		cere id-ul ca si get_book si trimite o cerere delete
		apeleaza timeout in caz de eroare

	logout:
		trimite un get
		nu apeleaza timeout in caz de eroare, desi teoretic cookie-ul ar fi
		putut sa expire inainte de triniterea cererii de logout pentru ca in
		main se vor "uita" oricum username-ul, cokie-ul si token-ul
		am ales sa le sterg in main pentru a trimite mai putini parametrii
		functiei logout
