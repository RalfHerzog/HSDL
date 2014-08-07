HTML-Semantic Definition Language.
==================================
Eine HSDL-Datei ist aufgebaut wie der HTML DOM mit zusätzlichen Attributen.

Verhalten:
Der Algorithmus betritt genau den Zweig der zu der SDL passt. Es wird überprüft ob der Elementname und die Attribute übereinstimmen. Desweiteren werden alle Elemente der gleichen Ebene überprüft sofern nicht durch HTML-SDL spezifizierte Attribute eingeschränkt.

HTML-SDL Dokumentdefinitionen
-----------------------------
Definitionen und Optionen können im ersten Element der HSDL-Datei angeben werden. Man definiert wie folgt:

	<?hsdl (Optionen)* ?>

Für Optionen lässt sich folgendes einsetzen:

    :output="(ini)" => Gibt die Ausgabe in folgendem Formaten zurück: INI, XML (geplant).
    
    :urlencode => Kodiert die Ausgabe mittels URL-Encoding. Die ist sinnvoll wenn der Inhalt Zeilenumbrüche oder Binärdaten hält.

HTML-SDL Attribute
------------------
:first => Trifft nur das erste Element.

:index="(+number)" => Trifft Element der position.

:sequence="((+number),)+" => Trifft nur auf gegebene Sequenz zu. Beispiel: ' :sequence="1,3,4" ' würde Element  1, 3 und 4 treffen.

:id => Übergibt eine ID für ein Element um zum Beispiel mehrere gleiche Elemente zu verarbeiten.

:interval="(+number)" => Trifft Elemente in einem Intervall. Beispiel: ' :interval="2" ' würde jedes zweite Element treffen.

:from="(+number)" => Trifft ab einem bestimmten index. Beispiel: ' :from="2" ' würde jedes Element außer dem ersten treffen.

:to="(+number)" => Trifft bis zu einem bestimmten index. Beispiel: ' :to="5" ' würde jedes Element bis zum fünften treffen.

:content => Extrahiert den vom Element eingeschlossenen Text (extraction).

:content_after => Extrahiert den vom Element nachfolgenden Text.

:textall => Extrahiert allen Text von Kind-Elementen.

:attribute="Attribut" => Extrahiert einen Attribut-Wert. Beispielsweise :attribute="href" gibt alle Verweise aus.

:varname="name" => Gibt einen Variablennamen aus wenn dieses Element verarbeitet wird. Ist :content definiert so wird als Ausgabe "name"="text" erzeugt.

:count => Fügt einen Zähler zu jeweiligem Variablennamen hinzu (zuzeit am Ende).

:extra="Daten" => Übergibt eine extradaten Zeichenkette an die Callbackfunktion. Nützlich um anzugeben ob die Daten noch konvertiert werden müssen.

:skip => Element wird bei der Verarbeitung übersprungen. Sinnvoll?

Für die Attribute :first, :index, :sequence, :interval, :from und :to gilt (alle mit Eingabe einer Zahl):
---------------------------------------------------------------------------------------------------------

Startelement hat index 0. Vergleiche mit for-Schleife "for( :from=0 ; :from < :to ; :from++ )"

Folgende Kombinationen sind zulässig:
-------------------------------------

:count wenn :varname, :content, :textall oder :attribute im Element gesetzt

:from + :to

:from + :interval

:from + :to + :interval

:to + :interval

Geplant (TODO)
--------------
:last => Trifft nur das letzte Element.

:from="(-number)" => Trifft ab einem bestimmten index. Beispiel: ' :from="-2" ' würde jedes Element vom vorletzten an treffen.

:to="(-number)" => Trifft bis zu einem bestimmten index. Beispiel: ' :to="-5" ' würde jedes Element bis zum fünftletzten treffen.

Äquivalenzen
------------
:first <=> :sequence="1"

:first <=> :to="1"

Programmaufruf
==============
Lokale Datei verarbeiten: cat DOM.html | ./HSDL schema.hsdl

Oder: ./HSDL schema.hsdl DOM.html

HTTP-Webseite: curl --output - http://www.example.org/ | ./HSDL schema.hsdl

Beispiele
=========

Beispiel 1:
-----------
HTML-SDL:

    <html>
    	<body>
    		<div :from="1" :to="3">
    			<a :content></a>
    		</div>
    	</body>
    </html>
    
HTML-DOM:

    <html>
    	<body>
    		<div>	<!-- Trifft nicht da index 0 und :from="1" -->
    			<a href="...">Link 1</a>
    		</div>
    		<div>	<!-- Trifft -->
    			<a href="...">Link 2</a>
    		</div>
    		<div>	<!-- Trifft -->
    			<a href="...">Link 3</a>
    		</div>
    		<div>	<!-- Trifft, aber es gibt kein <a> Element -->
    			<span>Link 4</span>
    		</div>
    	</body>
    </html>

Ausgabe von Beispiel 1:

    Link 2
    Link 3

Beispiel 2:
-----------
HTML-SDL:

    <?hsdl :output="ini" :urlencode ?>
    <html>
    	<body :varname="Links">
    		<div :from="1" :to="3">
    			<a :varname="Link" :count :content></a>
    		</div>
    	</body>
    </html>

HTML-DOM:

    <html>
    	<body>
    		<div>	<!-- Trifft nicht da index 0 und :from="1" -->
    			<a href="...">Link 1</a>
    		</div>
    		<div>	<!-- Trifft -->
    			<a href="...">Link 2</a>
    		</div>
    		<div>	<!-- Trifft -->
    			<a href="...">Link 3</a>
    		</div>
    		<div>	<!-- Trifft, aber es gibt kein <a> Element -->
    			<span>Link 4</span>
    		</div>
    	</body>
    </html>

Ausgabe von Beispiel 2:

    [Links]
    Link_0=Link%202
    Link_1=Link%203

Danksagung
==========
Besonderer Dank gebürt h4xxel und seinem html Parser auf dem HSDL aufbaut.