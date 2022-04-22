const char aqmIndex[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
    <head>
        <style>
            * {
                background-color: black;
                font-family: Arial, Helvetica, sans-serif
            }
            p, h1, label {
                color: white;
                font-size: 40px;
            }
            a {
                text-decoration: none;
                color: white;
            }
            button {
                background-color: black;
                color: white;
                border: solid white;
                margin: 10px;
                font-size: 40px;
                border-radius: 15px;
            }
            input {
                color: white;
                margin: 10px;
                font-size: 20px;
                border-radius: 15px;
            }
            #mainContainer {
                border: solid white;
                border-width: 10px;
                text-align: center;
                margin: 0 auto;
                padding: 10px;
            }
            #submitButton {
                background-color: black;
                color: white;
                border: solid white;
                margin: 10px;
                font-size: 20px;
                border-radius: 15px;
            }
            @media (pointer:none), (pointer:coarse) {
                p, h1, label {
                    font-size: 70px;
                }
                button {
                    font-size: 70px;
                }
                input {
                    font-size: 30px;
                }
                #submitButton {
                    font-size: 40px;
                }
            }
        </style>
    </head>
    <body>
        <section id="mainContainer">
            <h1>AQM-DEMO-MAIN</h1>
            <p><a href="/update">Update Page</a></p>
        </section>
    </body>
</html>
)=====";
const char updateIndex[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
    <head>
        <style>
            * {
                background-color: black;
                font-family: Arial, Helvetica, sans-serif
            }
            p, h1, label {
                color: white;
                font-size: 40px;
            }
            a {
                text-decoration: none;
                color: white;
            }
            button {
                background-color: black;
                color: white;
                border: solid white;
                margin: 10px;
                font-size: 40px;
                border-radius: 15px;
            }
            input {
                color: white;
                margin: 10px;
                font-size: 20px;
                border-radius: 15px;
            }
            #mainContainer {
                border: solid white;
                border-width: 10px;
                text-align: center;
                margin: 0 auto;
                padding: 10px;
            }
            #submitButton {
                background-color: black;
                color: white;
                border: solid white;
                margin: 10px;
                font-size: 20px;
                border-radius: 15px;
            }
            @media (pointer:none), (pointer:coarse) {
                p, h1, label {
                    font-size: 70px;
                }
                button {
                    font-size: 70px;
                }
                input {
                    font-size: 30px;
                }
                #submitButton {
                    font-size: 40px;
                }
            }
        </style>
        <script>
            function notify_update() {
                document.getElementById("update").innerHTML = "<p>Updating...</p>";
            }
        </script>
    </head>
    <body>
        <section id="mainContainer">
            <h1>AQM-DEMO-OTA</h1>
            <p>Only *.bin files will work!!</p>
            <form method='POST' action='/update' enctype='multipart/form-data' target='_self' onsubmit='notify_update()'>
                <input type='file' name='update' accept=".bin" required><br>
                <input id="submitButton" type='submit' value='Start Update!'>
            </form>
            <div id="update"></div>
        </section>
    </body>
</html>
)=====";
