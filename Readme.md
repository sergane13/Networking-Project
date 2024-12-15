# DigitalMenuBoard

## Prezentare Generală
Acest program implementează un server multithread care permite clienților să interacționeze cu un sistem de gestionare a meniului unui restaurant. Clienții pot:
- Accesa categorii specifice de meniuri.
- Actualiza categoriile de meniuri (doar dacă sunt autentificați ca manager).

Serverul utilizează **protocolul TCP** și ascultă conexiunile clienților pe portul 8080. Fiecare cerere a unui client este gestionată de un thread dedicat pentru a asigura concurența.

---

## Funcționalități Cheie
1. **Accesarea Meniului (`GET_MENU`)**  
   Clienții pot solicita conținutul unei categorii de meniu specificând numărul categoriei.

2. **Actualizarea Meniului (`CHANGE_MENU`)**  
   Utilizatorii autorizați (managerii) pot actualiza categoriile de meniu cu un conținut nou. Aceasta necesită autentificare.

3. **Autentificare (`AUTH`)**  
   Managerii se autentifică trimițând parola. Doar managerii autentificați pot actualiza meniul.

4. **Multithreading**  
   Fiecare conexiune a unui client este gestionată de un thread separat pentru a permite interacțiunea simultană a mai multor clienți cu serverul.

5. **Gestionarea Erorilor**  
   Serverul oferă răspunsuri pentru comenzi invalide, acțiuni neautorizate și formate de intrare incorecte.

---

## Detalii de Implementare

### Stocarea Meniului
Meniul restaurantului este stocat ca un array static de string-uri:
```c
char menu[5][100] = {
    "Pizza Vegana, Pizza Pui, Pizza Peperoni", 
    "Paste Carbonara, Paste cu Pesto, Paste alioli", 
    "Burger Vita, Burger Pui, Burger Crevete",
    "Sushi", 
    "Apa, Ceai, Cola, Pepsi"
};