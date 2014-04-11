package smart;

import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;
import smart.events.EventHandler;

public class Frame extends JFrame {

    private static final long serialVersionUID = -4985152588153565884L;
    public static final String USER_AGENT;
    private static final String TITLE = "SMART v8.1 Custom Revision";
    private static final String VERSION = "SMART v8.1";
    private static Client client = null;
    private static final Image trayIcon;
    private static final ImageIcon splashIcon, graphicsIcon, debugIcon, paintIcon, keyboardIcon, mouseIcon;

    static {
        String osname = System.getProperty("os.name");
        String windowing = "X11";
        if (osname.contains("Windows")) {
            windowing = "Windows";
        } else if (osname.contains("Mac")) {
            windowing = "Macintosh";
        }

        Utils Utility = Utils.getInstance();
        USER_AGENT = "Mozilla/5.0 (" + windowing + "; U; " + osname + " " + System.getProperty("os.version") + "; " + Locale.getDefault().getLanguage() + "-" + Locale.getDefault().getCountry() + "; rv:9.0.1) Gecko/20100101 Firefox/9.0.1";
        graphicsIcon = new ImageIcon(Utility.scaleImage(16, 16, Utility.loadResourceImage("/icons/Graphics.png")));
        paintIcon = new ImageIcon(Utility.scaleImage(16, 16, Utility.loadResourceImage("/icons/Painting.png")));
        debugIcon = new ImageIcon(Utility.scaleImage(16, 16, Utility.loadResourceImage("/icons/Debug.png")));
        keyboardIcon = new ImageIcon(Utility.scaleImage(16, 16, Utility.loadResourceImage("/icons/Keyboard.png")));
        mouseIcon = new ImageIcon(Utility.scaleImage(16, 16, Utility.loadResourceImage("/icons/Mouse.png")));
        splashIcon = Utility.loadResourceIcon("/icons/Loading.gif");
        trayIcon = Utility.loadResourceImage("/icons/SystemTray.png");
    }
    private JToolBar ToolBar = null;
    private JLabel splashScreen = null;
    private JComboBox<String> ModeSelector = null;
    private ConsolePanel consolePanel = new ConsolePanel();
    private JButton TrayBtn = null;
    private JToggleButton ConsoleBtn = null, GraphicsBtn = null, DebugBtn = null, MouseBtn = null, KeyboardBtn = null;
    private Console miniConsole = null;
    private boolean showMiniConsole = false;
    private boolean Minimized = false;
    private static SystemTray systemTray = null;
    private static TrayIcon systemTrayIcon;

    public Frame(int ID, boolean ShowMiniConsole) {
        this.showMiniConsole = ShowMiniConsole;
        this.initializeFrame(ID);
        this.addListeners();
    }

    private void initializeFrame(int ID) {
        this.setTitle(TITLE + "    ID: [" + ID + "]");
        JPopupMenu.setDefaultLightWeightPopupEnabled(false);
        this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        Utils.setCustomTheme(UIManager.getSystemLookAndFeelClassName());
        this.setLayout(new BorderLayout());
        this.setToolBar();
        this.setSystemTray();
        this.addSplashScreen();
        this.pack();
        this.centerWindow();
        this.setVisible(true);
    }

    public void printMiniConsole(String Value) {
        this.miniConsole.print(Value);
    }

    private void centerWindow() {
        Dimension ScreenSize = Toolkit.getDefaultToolkit().getScreenSize();
        int X = (int) ((ScreenSize.getWidth() - this.getWidth()) / 2);
        int Y = (int) ((ScreenSize.getHeight() - this.getHeight()) / 2);
        this.setLocation(X, Y);
    }

    private void addSplashScreen() {
        this.splashScreen = new JLabel(splashIcon);
        this.add(splashScreen, BorderLayout.CENTER);
        if (this.showMiniConsole) {
            this.add((this.miniConsole = new Console(this.getContentPane().getWidth(), 30, false)).getScrollPane(), BorderLayout.SOUTH);
        }
        this.pack();
    }

    private void removeSplashScreen() {
        if (this.splashScreen != null) {
            this.remove(this.splashScreen);
            this.splashScreen = null;
            this.consolePanel.Add(new Console(this.getContentPane().getWidth(), 75, true).getScrollPane(), "-1");
            this.add(consolePanel, BorderLayout.SOUTH);
            if (this.showMiniConsole) {
                this.remove(miniConsole.getScrollPane());
                this.miniConsole = null;
            }
        }
    }

    private void setSystemTray() {
        if (SystemTray.isSupported()) {
            systemTray = SystemTray.getSystemTray();

            PopupMenu trayMenu = new PopupMenu();
            MenuItem TrayMenuItem = new MenuItem("Show");
            TrayMenuItem.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    Frame.this.Minimized = false;
                    //Frame.this.setVisible(true);
                    JNI.Natives.addToTaskBar(JNI.Natives.getWindowHandle(Frame.this));
                    systemTray.remove(systemTrayIcon);
                    Frame.this.setState(JFrame.NORMAL);
                }
            });
            
            trayMenu.add(TrayMenuItem);
            TrayMenuItem = new MenuItem("Exit");
            TrayMenuItem.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    Frame.this.destruct();
                    System.exit(0);
                }
            });

            trayMenu.add(TrayMenuItem);     
            systemTrayIcon = new TrayIcon(trayIcon, VERSION, trayMenu);
            systemTrayIcon.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    Frame.this.Minimized = false;
                    //Frame.this.setVisible(true);
                    JNI.Natives.addToTaskBar(JNI.Natives.getWindowHandle(Frame.this));
                    systemTray.remove(systemTrayIcon);
                    Frame.this.setState(JFrame.NORMAL);
                }
            });
            systemTrayIcon.setImageAutoSize(true);
        }
    }

    private void setToolBar() {
        this.add(ToolBar = new JToolBar(), BorderLayout.NORTH);

        ToolBar.setLayout(new BoxLayout(ToolBar, BoxLayout.X_AXIS));
        ToolBar.add(ModeSelector = new JComboBox<>(new String[]{"SafeMode", "OpenGL", "DirectX"}));
        ModeSelector.setPreferredSize(new Dimension(10, 20));
        ToolBar.add(Box.createHorizontalGlue());
        ToolBar.add(Box.createHorizontalGlue());
        ToolBar.add(Box.createHorizontalGlue());
        ToolBar.add(TrayBtn = new JButton(new ImageIcon(Utils.getInstance().scaleImage(16, 16, trayIcon))));
        ToolBar.add(ConsoleBtn = new JToggleButton(debugIcon));
        ToolBar.add(GraphicsBtn = new JToggleButton(graphicsIcon));
        ToolBar.add(DebugBtn = new JToggleButton(paintIcon));
        ToolBar.add(MouseBtn = new JToggleButton(mouseIcon));
        ToolBar.add(KeyboardBtn = new JToggleButton(keyboardIcon));

        ToolBar.setFloatable(false);
        ToolBar.setFocusable(false);
        TrayBtn.setFocusable(false);
        ConsoleBtn.setFocusable(false);
        GraphicsBtn.setFocusable(false);
        DebugBtn.setFocusable(false);
        MouseBtn.setFocusable(false);
        KeyboardBtn.setFocusable(false);
        ModeSelector.setFocusable(false);

        ModeSelector.setSelectedIndex(0);
        Dimension Size = new Dimension(22, 22);
        TrayBtn.setPreferredSize(Size);
        ConsoleBtn.setPreferredSize(Size);
        GraphicsBtn.setPreferredSize(Size);
        DebugBtn.setPreferredSize(Size);
        MouseBtn.setPreferredSize(Size);
        KeyboardBtn.setPreferredSize(Size);

        TrayBtn.setToolTipText("Minimize To System Tray");
        ConsoleBtn.setToolTipText("Hide Console");
        GraphicsBtn.setToolTipText("Disable Graphics");
        DebugBtn.setToolTipText("Disable Painting");
        MouseBtn.setToolTipText("Disable Mouse");
        KeyboardBtn.setToolTipText("Disable Keyboard");
        ModeSelector.setToolTipText("Change Operation Mode");
    }

    public static void setClient(Client c) {
        client = c;
    }

    public static Client getClient() {
        return client;
    }

    public boolean isMinimized() {
        return Minimized;
    }

    public void setDebugConsole(boolean Visibility) {
        if (client != null) {
            Frame.this.consolePanel.Show(Visibility ? -1 : 0);
        }
    }

    public void updateUI() {
        if (client != null) {
            Frame.this.updateButtonStates(client);
            Frame.this.updateToolTips(client);
        }
    }

    private void updateToolTips(Client client) {
        KeyboardBtn.setToolTipText(client.isKeyboardEnabled() ? "Disable Keyboard" : "Enable Keyboard");
        MouseBtn.setToolTipText(client.isMouseEnabled() ? "Disable Mouse" : "Enable Mouse");
        GraphicsBtn.setToolTipText(client.isRenderBlocking() ? "Disable Graphics" : "Enable Graphics");
        DebugBtn.setToolTipText(client.isDebugFx() ? "Disable Painting" : "Enable Painting");
        ModeSelector.setToolTipText("Graphics Modes");
    }

    private void updateButtonStates(Client client) {
        KeyboardBtn.setSelected(!client.isKeyboardEnabled());
        KeyboardBtn.getModel().setPressed(!client.isKeyboardEnabled());
        MouseBtn.setSelected(!client.isMouseEnabled());
        MouseBtn.getModel().setPressed(!client.isMouseEnabled());

        GraphicsBtn.setSelected(client.isRenderBlocking());
        GraphicsBtn.getModel().setPressed(client.isRenderBlocking());
        DebugBtn.setSelected(client.isDebugFx());
        DebugBtn.getModel().setPressed(client.isDebugFx());
        ModeSelector.setSelectedItem(client.getOperatingMode().toString());
    }

    public void addClient(Client client) {
        this.removeSplashScreen();
        this.add(client.getLoader());
        this.consolePanel.Add(client.getLoader().getConsole().getScrollPane());
        this.consolePanel.Show(0);
        this.updateUI();
        this.pack();
        this.setSize(this.getWidth(), this.getHeight() + 1);
        this.centerWindow();
    }

    private void removeAllClients() {
        this.remove(client.getLoader());
        if (client != null && client.getLoader() != null && client.getCanvas() != null) {
            EventHandler.remove(client.getCanvas());
            client.getLoader().remove(client.getApplet());
            client.getLoader().destruct();
        }
        System.gc();
    }

    public void destruct() {
        Frame.this.setVisible(false);
        Frame.this.removeAllClients();
        Frame.this.dispose();
        Main.cleanUp();
        System.gc();
        System.exit(0);
    }

    private void addListeners() {
        this.addWindowListener(new WindowAdapter() {
            @Override
            public void windowActivated(WindowEvent e) {
                Frame.this.requestFocusInWindow();
            }

            @Override
            public void windowDeiconified(WindowEvent e) {
                Frame.this.requestFocusInWindow();
                Minimized = false;
            }

            @Override
            public void windowIconified(WindowEvent e) {
                Minimized = true;
            }

            @Override
            public void windowClosing(WindowEvent e) {
                Frame.this.destruct();
                System.exit(0);
            }
        });

        ModeSelector.addItemListener(new ItemListener() {
            @Override
            public void itemStateChanged(ItemEvent e) {
                if (e.getStateChange() == ItemEvent.SELECTED) {
                    if (client != null) {
                        client.setOperatingMode(OperatingMode.valueOf((String) ModeSelector.getSelectedItem()));
                    }
                }
            }
        });

        ConsoleBtn.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                ConsoleBtn.setToolTipText(ConsoleBtn.isSelected() ? "Show Console" : "Hide Console");
                if (client != null) {
                    Frame.this.consolePanel.setVisible(!ConsoleBtn.isSelected());
                    if (ConsoleBtn.isSelected()) {
                        Frame.this.setSize(Frame.this.getWidth(), Frame.this.getHeight() - 75);
                    } else {
                        Frame.this.setSize(Frame.this.getWidth(), Frame.this.getHeight() + 75);
                    }
                }
            }
        });

        GraphicsBtn.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (client != null) {
                    client.setGraphics(!client.isRenderBlocking());
                    DebugBtn.setEnabled(client.isRenderBlocking());
                }
            }
        });

        DebugBtn.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (client != null) {
                    client.setDebug(!client.isDebugFx());
                }
            }
        });

        KeyboardBtn.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (client != null) {
                    client.setKeyInput(!client.isKeyboardEnabled());
                }
            }
        });

        MouseBtn.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (client != null) {
                    client.setMouseInput(!client.isMouseEnabled());
                }
            }
        });
        
        TrayBtn.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (!Frame.this.Minimized) {
                    try {
                        if (SystemTray.isSupported()) {
                            systemTray.add(systemTrayIcon);
                            //Frame.this.setVisible(false);
                            JNI.Natives.removeFromTaskBar(JNI.Natives.getWindowHandle(Frame.this));
                            Frame.this.Minimized = true;
                        }
                        Frame.this.setState(JFrame.ICONIFIED);
                    } catch (Exception Ex) {
                        Main.StackTrace(Ex);
                    }
                }
            }
        });
    }
}
