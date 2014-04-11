package smart;

import java.awt.Desktop;
import java.awt.Dimension;
import java.io.*;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.HashMap;
import java.util.Map;
import javax.swing.*;
import javax.swing.event.HyperlinkEvent;
import javax.swing.event.HyperlinkListener;
import javax.swing.text.BadLocationException;
import javax.swing.text.DefaultCaret;
import javax.swing.text.html.HTMLDocument;
import javax.swing.text.html.HTMLEditorKit;

public class Console extends JTextPane {

    private static final long serialVersionUID = 7731036968185936516L;
    private JScrollPane ScrollPane = null;
    private final HTMLDocument StyleDocument = new HTMLDocument();
    private final HTMLEditorKit StyleEditorKit = new HTMLEditorKit();
    private static final ConsolePopUpMenu Menu = new ConsolePopUpMenu();

    public Console(int Width, int Height, boolean RedirectSystemStreams) {
        Dimension Size = new Dimension(Width, Height);
        this.ScrollPane = new JScrollPane(this, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
        this.setPreferredSize(Size);
        this.setMaximumSize(Size);
        this.ScrollPane.setPreferredSize(Size);
        this.ScrollPane.setMaximumSize(Size);
        this.setComponentPopupMenu(Menu);

        if (RedirectSystemStreams) {
            this.redirectSystemStreams();
        }

        this.setEditable(false);
        this.setContentType("text/html");
        this.setEditorKit(StyleEditorKit);
        this.setDocument(StyleDocument);
        ((DefaultCaret)this.getCaret()).setUpdatePolicy(DefaultCaret.ALWAYS_UPDATE);
        StyleDocument.getStyleSheet().addRule("a {color: #007898; text-decoration: underlined;}");

        this.addHyperlinkListener(new HyperlinkListener() {
            @Override
            public void hyperlinkUpdate(final HyperlinkEvent e) {
                if (HyperlinkEvent.EventType.ACTIVATED == e.getEventType()) {
                    Console.this.setEnabled(false);
                    Object[] Options = {"Yes", "No"};
                    int Result = JOptionPane.showOptionDialog(null, "Are you sure you want to open the URL: \n\n<html><font color=red>" + e.getURL().toString() + "</font>", "NOTICE!", JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE, null, Options, Options[1]);
                    if (Result == JOptionPane.OK_OPTION) {
                        try {
                            Desktop.getDesktop().browse(new URI(e.getURL().toString()));
                        } catch (IOException | URISyntaxException ex) {
                            Main.StackTrace(ex);
                        }
                    }
                    Console.this.setEnabled(true);
                }
            }
        });
    }

    public JScrollPane getScrollPane() {
        return ScrollPane;
    }

    public void setConsoleVisible(boolean Visible) {
        if (this.ScrollPane != null) {
            this.ScrollPane.setVisible(Visible);
        }
    }

    public boolean getDebugOptionState() {
        return Menu.getDebugOptionState();
    }

    private static String BBToHTML(String text) {
        HashMap<String, String> BBCodes = new HashMap<>();
        BBCodes.put("(\r\n|\r|\n|\n\r)", "<br/>");
        BBCodes.put("\\[b\\](.+?)\\[/b\\]", "<b>$1</b>");
        BBCodes.put("\\[i\\](.+?)\\[/i\\]", "<span style='font-style:italic;'>$1</span>");
        BBCodes.put("\\[u\\](.+?)\\[/u\\]", "<span style='text-decoration:underline;'>$1</span>");
        BBCodes.put("\\[center\\](.+?)\\[/center\\]", "<div align='center'>$1");
        BBCodes.put("\\[align=(.+?)\\](.+?)\\[/align\\]", "<div align='$1'>$2");
        BBCodes.put("\\[color=(.+?)\\](.+?)\\[/color\\]", "<span style='color:$1;'>$2</span>");
        BBCodes.put("\\[img\\](.+?)\\[/img\\]", "<img src='$1' />");
        BBCodes.put("\\[img=(.+?),(.+?)\\](.+?)\\[/img\\]", "<img width='$1' height='$2' src='$3' />");
        BBCodes.put("\\[url\\](.+?)\\[/url\\]", "<a href='$1'>$1</a>");
        BBCodes.put("\\[url=(.+?)\\](.+?)\\[/url\\]", "<a href='$1'>$2</a>");
        for (Map.Entry<String, String> entry : BBCodes.entrySet()) {
            text = text.replaceAll(entry.getKey().toString(), entry.getValue().toString());
        }
        return text;
    }

    public void print(String Text) {
        this.setOutputAreaText(Text);
    }

    public void clear() {
        this.setText(null);
    }

    private void setOutputAreaText(final String Text) {
        try {
            StyleEditorKit.insertHTML(StyleDocument, StyleDocument.getLength(), BBToHTML(Text).trim(), 0, 0, null);
        } catch (BadLocationException | IOException e) {
            Main.StackTrace(e);
        }
    }

    private void redirectSystemStreams() {
        OutputStream OutBuffer = new OutputStream() {
            @Override
            public void write(int value) throws IOException {
                Console.this.setOutputAreaText(String.valueOf((char) value));
            }

            @Override
            public void write(byte[] bytes, int offset, int length) throws IOException {
                Console.this.setOutputAreaText(new String(bytes, offset, length));
            }

            @Override
            public void write(byte[] bytes) throws IOException {
                write(bytes, 0, bytes.length);
            }
        };

        System.setOut(new PrintStream(OutBuffer, true));
        System.setErr(new PrintStream(OutBuffer, true));
    }
}
