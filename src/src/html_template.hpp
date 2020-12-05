
#include <Arduino.h>

const String html_text = R"EOF( 
<!DOCTYPE HTML>
<html>
<head>
    <title>
        Intercommie Remote
    </title>
    <style>
        body {
            background: darkslategrey;
            text-align: center;
        }
        button {
            margin: auto;
            width: 80%;
            transition-duration: 0.4s;
            background-color: green;
            border: none;
            padding: 120px 50px;
        }

        button:hover {
            background-color: lightgreen;
        }
    </style>
    <script type="text/javascript">
        var opener_interval = null;
        var closer_interval = null;

        function btn_down(){
            console.log("down");
            make_open_request();
            opener_interval = setInterval(notify_server, 1000);
        }

        function btn_up() {
            console.log("up");
            clearInterval(opener_interval)
            closer_interval = setInterval(make_close_request, 1000);
        }

        function notify_server() {
            console.log(".");
            make_open_request();
            
        }

        var httpreq = null
        function make_open_request() {
            if(httpreq == null || httpreq.readyState == 4){
                console.log("Sending new request");
                httpreq = new XMLHttpRequest();
                const url = 'https://84.114.113.52/relay=on';
                httpreq.open("GET", url)
                httpreq.send();

                httpreq.onreadystatechange = (e) => {
                    console.log(httpreq.responseText)
                }    
            }
        }

        function make_close_request() {
            if(httpreq == null || httpreq.readyState == 4){
                console.log("Sending new CLOSE request");
                httpreq = new XMLHttpRequest();
                const url = 'https://84.114.113.52/relay=off';
                httpreq.open("GET", url)
                httpreq.send();

                httpreq.onreadystatechange = (e) => {
                    console.log(httpreq.responseText)
                }
                clearInterval(closer_interval);    
            }
        }
    </script>
</head>
<body>
    <button type="button" id="btn_open" onmousedown="btn_down()" onmouseup="btn_up()" ontouchstart="btn_down()" ontouchend=btn_up()>Open</button>
</body>
</html>)EOF";