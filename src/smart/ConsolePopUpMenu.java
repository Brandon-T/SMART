package smart;

import java.awt.event.ActionEvent;

import javax.swing.*;
import javax.swing.text.TextAction;

public class ConsolePopUpMenu extends JPopupMenu {

    private static final long serialVersionUID = 8897580749118813744L;
    private static boolean DebugOptionState = false;

    public ConsolePopUpMenu() {
        JCheckBoxMenuItem Debug = new JCheckBoxMenuItem(new TextAction("Debug Console") {
            private static final long serialVersionUID = -336209978671944858L;

            @Override
            public void actionPerformed(ActionEvent e) {
                DebugOptionState = ((JCheckBoxMenuItem) e.getSource()).isSelected();
                Main.getFrame().setDebugConsole(DebugOptionState);
            }
        });

        JMenuItem Copy = new JMenuItem(new TextAction("Copy") {
            private static final long serialVersionUID = -6774461986513304498L;

            @Override
            public void actionPerformed(ActionEvent e) {
                if (ConsolePopUpMenu.this.getInvoker() instanceof JTextPane) {
                    ((JTextPane) ConsolePopUpMenu.this.getInvoker()).copy();
                }
            }
        });

        JMenuItem Clear = new JMenuItem(new TextAction("Clear") {
            private static final long serialVersionUID = -5567371173360543484L;

            @Override
            public void actionPerformed(ActionEvent e) {
                if (ConsolePopUpMenu.this.getInvoker() instanceof JTextPane) {
                    ((JTextPane) ConsolePopUpMenu.this.getInvoker()).setText(null);
                }
            }
        });

        JMenuItem SelectAll = new JMenuItem(new TextAction("Select All") {
            private static final long serialVersionUID = -8792250195980016624L;

            @Override
            public void actionPerformed(ActionEvent e) {
                if (ConsolePopUpMenu.this.getInvoker() instanceof JTextPane) {
                    ((JTextPane) ConsolePopUpMenu.this.getInvoker()).selectAll();
                }
            }
        });

        this.add(Copy);
        this.add(Clear);
        this.add(SelectAll);
        this.add(Debug);
    }

    public boolean getDebugOptionState() {
        return DebugOptionState;
    }
}
