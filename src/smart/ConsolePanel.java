package smart;

import java.awt.CardLayout;
import java.awt.Component;
import javax.swing.*;

public class ConsolePanel extends JPanel {

    private int indexCount = 0;
    private CardLayout cardLayout = new CardLayout();

    public ConsolePanel() {
        this.setLayout(cardLayout);
    }

    public void Add(Component component) {
        this.add(component, String.valueOf(indexCount++));
    }

    public void Add(Component component, String value) {
        this.add(component, value);
    }

    public void Remove(Component component) {
        this.remove(component);
    }

    public void Remove(int index) {
        this.remove(index);
    }

    public void Show(int index) {
        this.cardLayout.show(this, String.valueOf(index));
    }

    public void Show(String value) {
        this.cardLayout.show(this, value);
    }
}

