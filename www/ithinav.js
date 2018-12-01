function initnav() {
    document.getElementById("navMenu").innerHTML =
        '<div class="navbar">' +
        '<a href="index.html">Home</a>' +
        '<div class="dropdown">' +
        '<a href="index.html">Metrics</a>' +
        '<div class="dropdown-content">' +
        '<a href="graph-m1.html">M1: Whois</a>' +
        '<a href="graph-m2.html">M2: Abuses</a>' +
        '<a href="graph-m3.html">M3: Root Servers</a>' +
        '<a href="graph-m4.html">M4: Recursive Servers</a>' +
        '<a href="graph-m5.html">M5: Recursive Resolver Integrity</a>' +
        '<a href="graph-m6.html">M6: IANA</a>' +
        '<a href="graph-m7.html">M7: DNSSEC</a>' +
        '<a href="graph-m8.html">M8: Authoritative</a>' +
        '</div>' +
        '</div>' +

        '<div class="dropdown">' +
        '<a href="participating.html">Participate</a>' +
        '<div class="dropdown-content">' +
        '<a href="participating.html">Participate</a>' +
        '<a href="partners/partnersOnly.html">Partners Only</a>' +
        '</div>' +
        '</div>' +

        '<div class="dropdown">' +
        '<a href="about.html">About</a>' +
        '<div class="dropdown-content">' +
        '<a href="about.html">About ITHI</a>' +
        '<a href="about-m1.html">About M1</a>' +
        '<a href="about-m2.html">About M2</a>' +
        '<a href="about-m3.html">About M3</a>' +
        '<a href="about-m4.html">About M4</a>' +
        '<a href="about-m5.html">About M5</a>' +
        '<a href="about-m6.html">About M6</a>' +
        '<a href="about-m7.html">About M7</a>' +
        '<a href="about-m8.html">About M8</a>' +
        '</div>' +
        '</div>' +
        '</div>';
}
