# Regole d'esame
L'esame finale consiste di due parti:

Presentazione e discussione di un progetto sviluppato in autonomia da un singolo studente;
Prova orale, che verterà principalmente sulla parte di programma non compresa nelle conoscenze necessarie allo sviluppo del progetto.
Il progetto deve essere consegnato per la valutazione almeno una settimana prima dell'esame (o più, su indicazione dei docenti, qualora le prenotazioni per un dato appello siano particolarmente numerose).

La discussione del progetto si fa insieme alla prova orale e consiste in una presentazione (basata su slide o sulla relazione di accompagnamento, a scelta dello studente). 

Leggere e seguire con attenzione tutte le regole indicate nel seguito. Il mancato rispetto delle regole può portare non solo al non superamento dell'esame, ma anche all'invalidazione del progetto, con necessità di svilupparne un altro ex novo. 

Dettagli sul progetto
Il progetto può riguardare, in alternativa:

Lo sviluppo di un'applicazione di grafica interattiva 2D, utilizzando la libreria SFML;
Lo sviluppo di un'applicazione di grafica 3D, utilizzando le librerie SFML (per l'interazione), OpenGL (per la grafica) e GLM (per i conti).
Le seguenti prescrizioni sono obbligatorie:

Ogni studente deve presentare preliminarmente, tramite email a entrambi i docenti, una breve descrizione informale del progetto che intende sviluppare, elencandone le caratteristiche e funzionalità. È possibile, ma non obbligatorio, allegare schizzi e immagini esemplificative. 
I docenti possono chiedere modifiche al piano, per renderlo adeguato (né troppo semplice, né troppo ambizioso).
Lo studente deve sviluppare il progetto secondo il piano concordato, consegnando sull'apposito modulo AulaWeb un repository Git contenente l'intero progetto (o, ancor meglio, un file testo contenente il link a un repository Git pubblico). Il repository deve contenere, in particolare:
Tutti i file necessari per la configurazione, la compilazione e l'esecuzione, inclusi eventuali risorse e file dati
Almeno 10 versioni del progetto a stadi di sviluppo successivi, coerenti tra loro, tutte compilabili ed eseguibili autonomamente
La documentazione necessaria a guidare l'utente nel processo di build, nel lanciare gli eseguibili e nell'uso dell'applicazione. Deve essere un file README.md (markdown) nella root di progetto.
Una relazione non troppo lunga, che spieghi nel dettaglio le varie fasi di sviluppo consegnate, le difficoltà incontrate e le soluzioni adottate. È obbligatorio segnalare qualunque risorsa o parte di codice non sviluppata autonomamente, indicandone la fonte, pena l'invalidazione del progetto.
Il progetto deve essere obbligatoriamente sviluppato in C++ e compilabile con CMake sui principali sistemi operativi (Linux, MacOS, Windows). Si raccomanda di partire dal template fornito per i laboratori, eventualmente  estendendolo senza stravolgerne la struttura. I progetti che non compilano non saranno valutati. 
È consentito l'uso delle seguenti librerie (oltre, naturalmente, alla standard library C++):
SFML 3.0 per i progetti di grafica 2D
SFML 3.0, OpenGL 4.1, glad e GLM per i progetti di grafica 3D.
Non sono ammessi progetti che usino versioni di SFML o OpenGL diverse da quelle indicate. Chi desiderasse utilizzare anche altre librerie, cosa che in alcuni casi può anche essere conveniente o opportuna, deve concordarlo preventivamente coi docenti. 

Consultare il forum tecnico per ulteriori dettagli.

Consegna
Consegnate un file .zip del repository Git o, meglio, un file txt contenente il link a un repository Git pubblico.

Per la struttura del progetto e l'organizzazione in tappe ci sono due strade suggerite, una descritta qui di seguito e fatta manualmente, un'altra tramite git tags e script di supporto, che trovate insieme alla sua documentazione nella cartella Laboratori/Risorse.

Se non volete seguire le strade suggerite, potete anche organizzare il codice a modo vostro, a patto che siano rispettate le seguenti condizioni:

1) Che il codice di progetto sia versionato con git.
2) Che le tappe di sviluppo nella consegna siano tutte accessibili contemporaneamente, e compilabili con un solo comando.
3) Se consegnate uno zip con il repository, consegnate repository puliti: cartelle o file ignorati da gitignore vanno ripuliti.

Se siete in dubbio,  seguite una delle strade suggerite, o consultate i docenti per chiedere se quello che avete pensato va bene.

Se volete creare le tappe manualmente, l'organizzazione in file dovrà avere la struttura seguente:

cartella principale del progetto
CMakeLists.txt
Cartella-risorse (comune per tutte le tappe)
file texture ecc...
Cartella-Tappa01
file sorgente della tappa1
Cartella-Tappa02
file sorgente tappa2
...
Cartella-TappaN
file sorgente tappaN
Il file CMakeLists.txt può essere clonato da quello utilizzato per i laboratori, cancellando alla fine tutti i comandi

add_executable(....)

target_link_libraries(....)

e aggiungendo per ogni tappa X:

add_executable(TappaX   Cartella-TappaX/file1.cpp Cartella-TappaX/file2.cpp ....) dove file1, file2 ecc sono tutti i .cpp che vanno compilati per quella tappa, tutti presenti nella cartella TappaX insieme ai relativi file .h

target_link_libraries(TappaX  PRIVATE  SFML::Graphics)

Ogni file sorgente che ha bisogno delle risorse le dovrà cercare col path relativo ../cartella-risorse/nomefile

Non inserite per nessun motivo path assoluti.

Semplicemente iniziate il progetto con la sola cartella Tappa01. Quando decidete di salvarla, fatene una copia, la rinominate Tappa02 e iniziate a lavorare in quella... e così via. Ogni volta dovete modificare il CMakeLists.txt aggiungendo il nuovo target. 

Nella cartella build si troveranno tutti gli eseguibili di tutte le tappe (ma questa non va consegnata)

Questo vale sia che facciate una repository git, sia che consegniate un unico .zip contenente la cartella principale.

Documentazione:  un solo file README.md nella root di progetto, che descrive:

Come fare build di tutte le tappe. Come da requisiti, deve essere un metodo unico che compila tutte le tappe insieme.
Le linee di comando per lanciare gli eseguibili (potenzialmente diverse per le varie tappe, se queste aggiungono modifiche alla sintassi degli argomenti, se  richiedono di passare al programma risorse aggiuntive, etc).
Elenco schematico dei comandi dell' interfaccia utente. Cosa si può fare con il mouse, cosa si può fare con la tastiera (anche quello per tappe, laddove le tappe aggiungono funzionalità e modifiche all'interfaccia utente).
Relazione: File pdf o markdown, che spieghi:

Cosa si propone di fare il progetto.
Cosa aggiunge o risolve ciascuna tappa.
Problemi riscontrati (se ci sono).
Soluzioni tecniche adottate (se rilevanti).
Eventuale codice esterno utilizzato (preso da tutorial, risorse in rete, AI o altro).
Se usare screenshot e/o immagini nella relazione vi può aiutare a descrivere le cose in modo più immediato, fatelo.
Potete consegnare un file unico, o un file per tappa.
Presentazione:  potete fare come volete: con slide, senza slide, usando la relazione che avete consegnato come canovaccio, presentando le tappe del progetto con delle live demo... Come vi viene meglio. L'obiettivo è raccontarci cosa volevate fare, le fasi di sviluppo, le soluzioni tecniche più importanti o interessanti che avete adottato, e il risultato finale raggiunto.

Ultime modifiche: mercoledì, 27 maggio 2026, 00:15