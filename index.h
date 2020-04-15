const char PAGE_INDEX[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
    <head>
        <meta charset="utf-8">
        <title>小轩010的ESP8266</title>
    </head>

    <body>
        <h3>ESP8266 设置</h3>
        <h4>闹钟</h4>
        <form method="POST" action="/settings/alarm" enctype="multipart/form-data">
            <p>闹钟：
                <input type="radio" name="isAlarmOn" value="true" />开
                <input type="radio" name="isAlarmOn" value="false" />关
            </p>
            <p>闹钟时间：<input type="text" name="alarmTime" \></p>
            <input type="submit" value="确定">
        </form>
        <br/>
        <h4>显示器</h4>
            <form method="POST" action="/settings/display" enctype="multipart/form-data">
                <p>显示屏：
                    <input type="radio"  name="isSavePower" value="0" />开
                    <input type="radio"  name="isSavePower" value="1" />关
                    <input type="submit" value="确定">
                </p>
            </form>
        <br/><br/>
        <h3>ESP8266 固件上传</h3>
        <form method="POST" action="/update" enctype="multipart/form-data">
            <p>请选择固件文件：
                <input type="file" accept=".bin" name="update"  />
                <input type="submit" value='上传'>
            </p>
        </form>
    </body>
</html>
)=====";
