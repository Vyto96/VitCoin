# VitCoin
Simulation of a cryptocurrency (VitCoin) for the examination of Computer Networks for the Parthenope University of 
Naples. 
The exam track includes the following specifications (original track in Italian):



Laboratorio di Reti di Calcolatori
A.A. 2018 - 2019
BitCoin
Descrizione
Si vuole realizzare un sistema per la gestione di una criptovaluta basato su
una rete P2P. Il sistema è basato sulla gestione di una blockchain, ovvero
una sequenza di blocchi in cui ogni blocco contiene una transazione.
Il sistema si compone di due tipi di nodi: NodiN e NodiW. I NodiN creano
la rete P2P e gestiscono la blockchain. Inoltre stampano la blockchain ogni
volta che viene aggiunto un blocco: (blocco 1 )− > (blocco 2 )− > (blocco 3 )− >
(blocco 4 ). I NodiW gestiscono i wallet (portafogli virtuali) che consentono di
inviare e ricevere pagamenti. Ad ogni nuovo pagamento inviato o ricevuto
il nodo stampa la transazione ed il totale del portafogli.
Si utilizzi il linguaggio C o Java su piattaforma UNIX utilizzando i socket
per la comunicazione tra processi. Corredare l’implementazione di adeguata
documentazione.


Descrizione dettagliata

NodoW:
Un NodoW per ricevere un pagamento fornisce il proprio nome (IP PORTA)
ed attende di ricevere il blocco contenente la transazione corrispondente. Ri-
cevuta la transazione si somma l’ammontare al totale del wallet. Per effet-
tuare un pagamento si crea una transazione IP PORTA MITT:AMMONTA
1RE:IP PORTA DEST:NUMERO RANDOM e si invia ad un NodoN cui il
NodoW è connesso.

NodoN:
Ogni NodoN gestisce una copia della blockchain, una sequenza di blocchi in
cui ogni blocco contiene una transazione. Un blocco contiene: il numero di
blocco progressivo, tempo di attesa random, transazione (IP PORTA MITT:
AMMONTARE:IP PORTA DEST:NUMERO RANDOM). Una blockchain
inizia con un blocco genesi, ovvero un blocco uguale per tutti i NodiN. Un
NodoN che riceve una transazione, la memorizza in un blocco, attende un
tempo random (in [5,15] sec) che inserisce nel blocco, inserisce il blocco in
testa alla blockchain e lo invia a tutti i NodiN e NodiW connessi. Un NodoN
che riceve un nuovo blocco lo inserisce in testa alla blockchain e se si trova
nella fase di attesa per l’inserimento di un blocco con lo stesso numero pro-
gressivo, invalida il proprio blocco, crea un nuovo blocco e tenta nuovamente
l’inserimento. Nel caso in cui un NodoN ricevesse un blocco con lo stesso
numero progressivo del blocco in testa alla blockchain, lo inserirebbe allo
stesso livello della testa (in questo modo in testa alla blockchain ci possono
essere più blocchi diversi). Nel caso in cui si dovesse aggiungere un blocco
ad una blockchain con più blocchi in testa, si sceglierebbe il blocco che pre-
senta la maggiore somma dei tempi di attesa random a partire dal primo.

Opzionale: nel caso di aggiunta di un blocco ad una blockchain con più nodi
in testa, dopo aver aggiunto il nuovo blocco dopo il blocco che presenta la
maggiore somma dei tempi di attesa random a partire dal primo, rimuovere
tutti gli altri blocchi dello stesso livello. Il nodo che aveva per primo ag-
giunto un blocco duplicato ha la responsabilità di creare un nuovo blocco e
tentare nuovamente l’inserimento.
