const { app, BrowserWindow, screen } = require('electron')

function getRatio(display) {
    return display.bounds.height / display.bounds.width;
}

app.on('ready', () => {

    let bigWidth, bigHeight, smallWidth, smallHeight;
    let displays = screen.getAllDisplays();
    displays.forEach((display) => {
        let ratio = getRatio(display);
        if(ratio > 2.0) {
            bigWidth = display.bounds.width;
            bigHeight = display.bounds.height;
        } else {
            smallWidth = display.bounds.width;
            smallHeight = display.bounds.height;
        }
    });

    console.log(bigWidth, bigHeight, smallWidth, smallHeight);

    const win = new BrowserWindow({width: bigWidth, height: bigHeight * 2, x: smallWidth, y: 0, frame: false});
    win.loadFile('index.html')
    win.show();
});