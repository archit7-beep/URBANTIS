import electron from 'electron';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const { app, BrowserWindow, Menu } = electron;

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

let mainWindow;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1540,
    height: 940,
    minWidth: 1100,
    minHeight: 720,
    title: 'URBANTIS — Pune Smart City Operations & Cyber-Resilience',
    backgroundColor: '#121413',
    autoHideMenuBar: true,
    titleBarStyle: 'hidden',
    titleBarOverlay: {
      color: '#121413',
      symbolColor: '#A3B899',
      height: 48
    },
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
    },
    show: false
  });

  // Remove the default OS top menu bar entirely (no File/View/Help black bar)
  Menu.setApplicationMenu(null);

  // Pure local application bundle - zero localhost web servers or network dependencies
  const appPath = path.join(__dirname, 'dist', 'index.html');
  mainWindow.loadFile(appPath);

  mainWindow.once('ready-to-show', () => {
    mainWindow.show();
  });

  mainWindow.on('closed', () => {
    mainWindow = null;
  });
}

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});
