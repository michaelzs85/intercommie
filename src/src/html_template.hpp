
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
        var open_request_in_progress = false;
        var close_request_in_progress = false;
        var do_a_close_request = false;

        function btn_down(){
            console.log("down");
            make_open_request();
            opener_interval = setInterval(notify_server, 1000);
        }

        function btn_up() {
            console.log("up");
            clearInterval(opener_interval)
            make_close_request();
            //do_a_close_request = true;
        }

        function notify_server() {
            console.log(".");
            make_open_request();
            
        }

        function make_open_request() {
            if(open_request_in_progress) {
                console.log("An open request is still in progress. Won't send another one!");
                return;
            }
            console.log("Sending new request");
            var httpreq = new XMLHttpRequest();
            // const url = 'https://84.114.113.52/relay=on';
            const url = 'https://10.0.0.44/relay=on';
            httpreq.open("POST", url)
            httpreq.send();
            open_request_in_progress = true;

            httpreq.onreadystatechange = (e) => {
                open_request_in_progress = false;
                console.log(httpreq.responseText);
                if(do_a_close_request) {
                    do_a_close_request = false;
                    make_close_request();
                }
            }    
        }

        function make_close_request() {
            if(close_request_in_progress) {
                console.log("Close request in progress. Not sending another one.")
                return;
            }
            console.log("Sending new CLOSE request");
            var httpreq = new XMLHttpRequest();
            // const url = 'https://84.114.113.52/relay=off';
            const url = 'https://10.0.0.44/relay=off';
            httpreq.open("POST", url)
            httpreq.send();

            httpreq.onreadystatechange = (e) => {
                close_request_in_progress = false;
                console.log(httpreq.responseText)
            }
        }
    </script> 
</head>
<body>
    <button type="button" id="btn_open" onmousedown="btn_down()" onmouseup="btn_up()" ontouchstart="btn_down()" ontouchend="btn_up()" ontouchcancel="btn_up()">Open</button>
</body>
</html>)EOF";