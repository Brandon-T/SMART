package java.awt;

import java.awt.image.BufferStrategy;
import java.awt.peer.CanvasPeer;
import javax.accessibility.*;
import smart.Client;
import smart.Main;

public class Canvas extends Component implements Accessible {

    private Client client = null;
    private boolean Focused = false;
    private static int nameCounter = 0;
    private static final String base = "canvas";
    private static final long serialVersionUID = -2284879212465893870L;

    public Canvas() {
        super();
    }

    public Canvas(GraphicsConfiguration config) {
        this();
        setGraphicsConfiguration(config);
    }

    private void updateClientInfo() {
        if (this.client == null) {
            this.client = smart.Frame.getClient();
        }
    }

    @Override
    public Graphics getGraphics() {
        this.updateClientInfo();
        if (client != null && client.getGameBuffer() != null) {
            try {
                return client.drawGraphics((Graphics2D) super.getGraphics());
            } catch (NoSuchFieldException | IllegalArgumentException | IllegalAccessException Ex) {
                Main.debug(Ex.getMessage());
            }
        }
        return super.getGraphics();
    }

    @Override
    public void paint(Graphics g) {
        g.clearRect(0, 0, width, height);
    }

    @Override
    public void update(Graphics g) {
        g.clearRect(0, 0, width, height);
        paint(g);
    }

    @Override
    public final boolean hasFocus() {
        return Focused;
    }

    @Override
    public final boolean isValid() {
        return visible;
    }

    @Override
    public final boolean isVisible() {
        return visible;
    }

    @Override
    public final boolean isDisplayable() {
        return true;
    }

    @Override
    public final void setVisible(boolean visible) {
        super.setVisible(visible);
        this.visible = visible;

        if (this.client == null) {
            this.client = smart.Frame.getClient();
        }

        if (this.client != null) {
            this.client.updateCanvas(this);

            switch (this.client.getOperatingMode()) {
                case OpenGL:
                    try {
                        this.client.drawGraphics(null);
                    } catch (NoSuchFieldException | IllegalArgumentException | IllegalAccessException Ex) {
                        Main.StackTrace(Ex);
                    }
                    break;

                case DirectX:
                    try {
                        this.client.drawGraphics(null);
                    } catch (NoSuchFieldException | IllegalArgumentException | IllegalAccessException Ex) {
                        Main.StackTrace(Ex);
                    }
                    break;

                default:
                    break;
            }
        }
    }

    @Override
    void setGraphicsConfiguration(GraphicsConfiguration gc) {
        synchronized (getTreeLock()) {
            CanvasPeer peer = (CanvasPeer) getPeer();
            if (peer != null) {
                gc = peer.getAppropriateGraphicsConfiguration(gc);
            }
            super.setGraphicsConfiguration(gc);
        }
    }

    @Override
    String constructComponentName() {
        synchronized (Canvas.class) {
            return base + nameCounter++;
        }
    }

    @Override
    public void addNotify() {
        synchronized (getTreeLock()) {
            if (peer == null) {
                peer = getToolkit().createCanvas(this);
            }
            super.addNotify();
        }
    }

    @Override
    boolean postsOldMouseEvents() {
        return true;
    }

    @Override
    public void createBufferStrategy(int numBuffers) {
        super.createBufferStrategy(numBuffers);
    }

    @Override
    public void createBufferStrategy(int numBuffers,
            BufferCapabilities caps) throws AWTException {
        super.createBufferStrategy(numBuffers, caps);
    }

    @Override
    public BufferStrategy getBufferStrategy() {
        return super.getBufferStrategy();
    }

    @Override
    public AccessibleContext getAccessibleContext() {
        if (accessibleContext == null) {
            accessibleContext = new Canvas.AccessibleAWTCanvas();
        }
        return accessibleContext;
    }

    protected class AccessibleAWTCanvas extends Component.AccessibleAWTComponent {

        private static final long serialVersionUID = -6325592262103146699L;

        @Override
        public AccessibleRole getAccessibleRole() {
            return AccessibleRole.CANVAS;
        }
    }
}
