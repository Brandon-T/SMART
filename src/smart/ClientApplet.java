package smart;

import java.applet.*;
import java.awt.*;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.*;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

public final class ClientApplet extends JPanel implements AppletStub {

    private static final long serialVersionUID = 5627929030422355843L;
    private Applet applet = null;
    private Console console = null;
    private URLClassLoader ClassLoader = null;
    private URL codeBase = null, documentBase = null;
    private CardLayout Layout = new CardLayout(0, 0);
    private HashMap<String, String> parameters = new HashMap<>();
    private static final Pattern codeRegex = Pattern.compile("code=(.*) ");
    private static final Pattern archiveRegex = Pattern.compile("archive=(.*) ");
    private static final Pattern parameterRegex = Pattern.compile("<param name=\"([^\\s]+)\"\\s+value=\"([^>]*)\">");

    public ClientApplet(String root, String params, int Width, int Height) {
        this(root, params, null, Width, Height);
    }

    public ClientApplet(String root, String params, String UserAgent, int Width, int Height) {
        try {
            String Address = root + params;
            this.setLayout(new BorderLayout(0, 0));
            String pageSource = downloadPage(new URL(Address), UserAgent);
            Matcher codeMatcher = codeRegex.matcher(pageSource);
            Matcher archiveMatcher = archiveRegex.matcher(pageSource);

            if (codeMatcher.find() && archiveMatcher.find()) {
                String archive = archiveMatcher.group(1);
                String jarLocation = root + archive;
                String code = codeMatcher.group(1).replaceAll(".class", "");
                Matcher parameterMatcher = parameterRegex.matcher(pageSource);
                this.codeBase = new URL(root + archive);
                this.documentBase = new URL(root);

                while (parameterMatcher.find()) {
                    this.parameters.put(parameterMatcher.group(1), parameterMatcher.group(2));
                }

                this.ClassLoader = new URLClassLoader(new URL[]{new URL(jarLocation)});
                this.applet = (Applet) ClassLoader.loadClass(code).newInstance();
                this.applet.setStub(this);
                this.applet.setPreferredSize(new Dimension(Width, Height));
                this.add(this.applet, BorderLayout.CENTER);
                this.console = new Console(Width, 75, false);
            }
        } catch (IOException | ClassNotFoundException | InstantiationException | IllegalAccessException Ex) {
            Main.StackTrace(Ex);
            JOptionPane.showMessageDialog(null, "Error Loading.. Please Check Your Internet Connection.", "Error Loading..", JOptionPane.ERROR_MESSAGE);
            Main.getFrame().destruct();
        }
    }

    public void printConsole(String Text) {
        if (this.console != null) {
            this.console.print(Text);
        }
    }

    public void clearConsole() {
        if (this.console != null) {
            this.console.clear();
        }
    }
    
    public Console getConsole() {
        return this.console;
    }

    public static String downloadPage(URL Address, String UserAgent) throws IOException {
        try {
            URLConnection Connection = Address.openConnection();
            Connection.addRequestProperty("Protocol", "HTTP/1.1");
            Connection.addRequestProperty("Connection", "keep-alive");
            Connection.addRequestProperty("Keep-Alive", "300");
            if (UserAgent != null) {
                Connection.addRequestProperty("User-Agent", UserAgent);
            } else {
                Connection.addRequestProperty("User-Agent", "Mozilla/5.0 (" + System.getProperty("os.name") + " " + System.getProperty("os.version") + ") Java/" + System.getProperty("java.version"));
            }
            byte[] Buffer = new byte[Connection.getContentLength()];
            try (DataInputStream Stream = new DataInputStream(Connection.getInputStream())) {
                Stream.readFully(Buffer);
            }
            return new String(Buffer);
        } catch (Exception Ex) {
            Main.StackTrace(Ex);
            return null;
        }
    }

    public Applet getApplet() {
        return this.applet;
    }

    public void start() {
        if (this.applet != null) {
            this.applet.init();
        }

        if (this.applet != null) {
            this.applet.start();
        }
    }

    public void destruct() {
        if (this.applet != null) {
            this.remove(this.applet);
            this.applet.stop();
            this.applet.destroy();
            this.applet = null;
        }

        if (this.ClassLoader != null) {
            try {
                this.ClassLoader.close();
            } catch (IOException e) {
                Main.StackTrace(e);
            }
        }
    }

    @Override
    public boolean isActive() {
        return true;
    }

    @Override
    public URL getDocumentBase() {
        return this.documentBase;
    }

    @Override
    public URL getCodeBase() {
        return this.codeBase;
    }

    @Override
    public String getParameter(String name) {
        return this.parameters.get(name);
    }

    @Override
    public AppletContext getAppletContext() {
        return null;
    }

    @Override
    public void appletResize(int width, int height) {
    }
}