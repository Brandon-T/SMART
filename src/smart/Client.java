package smart;

import JNI.GLDXLoader;
import java.applet.Applet;
import java.awt.*;
import java.awt.image.*;
import java.lang.reflect.Field;
import java.nio.*;
import static smart.OperatingMode.*;

public class Client {
    public int transColor = 0;
    public boolean active = false;
    private boolean drawMouse = false;
    private OperatingMode Mode = null;
    private ClientApplet loader = null;
    private String initSequence = null;
    private InputManager inputManager = null;
    private boolean renderWhileBlocking = true, debugfx = false;
    private IntBuffer gameBuffer = null, debugBuffer = null;
    private BufferedImage gameImage = null, debugImage = null;
    
    public Client(String root, String params, String initSequence, String userAgent, ByteBuffer gameBuffer, ByteBuffer debugBuffer, int width, int height) {
        loader = new ClientApplet(root, params, userAgent, width, height);
        this.initSequence = initSequence;
        this.gameBuffer = gameBuffer.order(ByteOrder.LITTLE_ENDIAN).asIntBuffer();
        this.debugBuffer = debugBuffer.order(ByteOrder.LITTLE_ENDIAN).asIntBuffer();
        this.gameImage = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
        this.debugImage = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
        this.Mode = OperatingMode.SafeMode;
        this.inputManager = new InputManager();
    }
    
    public void start() {
        this.loader.start();
        while (this.getCanvas() == null) {
            Utils.sleep(10);
        }
        
        Main.getFrame().setClient(this);   
        while (!this.inputManager.isInputReady()) {
            Utils.sleep(100);
        }
        
        if (this.initSequence != null) {
            this.inputManager.sendKeys(this.initSequence, 2000, 60);
            this.initSequence = null;
        }
        
        this.active = true;
    }
    
    public Applet getApplet() {
        return loader != null ? loader.getApplet() : null;
    }

    public Canvas getCanvas() {
        if (loader.getApplet() == null || loader.getApplet().getComponentCount() < 1) {
            return null;
        }
        return (Canvas) loader.getApplet().getComponent(0);
    }
    
    public void setDrawMouse(boolean drawMouse) {
        this.drawMouse = drawMouse;
    }

    public BufferedImage getGameBuffer() {
        return this.gameImage;
    }

    public void updateCanvas(Canvas canvas) {
        this.inputManager.update(this, canvas);
    }
    
    public ClientApplet getLoader() {
        return this.loader;
    }

    public void setOperatingMode(OperatingMode Mode) {
        this.Mode = Mode;
    }

    public OperatingMode getOperatingMode() {
        return this.Mode;
    }
    
    public boolean isRenderBlocking() {
        return renderWhileBlocking;
    }

    public boolean isDebugFx() {
        return debugfx;
    }

    /* All of the below are Convenience methods for JNI written by Benland100 with some modifications */
    public void moveMouse(int x, int y) {
        if (!this.isMouseEnabled()) {
            this.inputManager.moveMouse(x, y);
            
            if (GLDXLoader.OpenGLLoaded || GLDXLoader.DirectXLoaded) {
                GLDXLoader.DrawMouse(x, y, 0, 255, 0);
            }
        }
    }

    public void windMouse(int x, int y) {
        if (!this.isMouseEnabled()) {
            this.inputManager.windMouse(x, y);
            
            if (GLDXLoader.OpenGLLoaded || GLDXLoader.DirectXLoaded) {
                GLDXLoader.DrawMouse(x, y, 0, 255, 0);
            }
        }
    }

    public void clickMouse(int x, int y, int button) {
        this.inputManager.clickMouse(x, y, button);
    }

    public void holdMouse(int x, int y, int button) {
        this.inputManager.holdMouse(x, y, button);
    }

    public void releaseMouse(int x, int y, int button) {
        this.inputManager.releaseMouse(x, y, button);
    }

    public boolean isMouseButtonHeld(int button) {
        return this.inputManager.isMouseButtonHeld(button);
    }

    public Point getMousePos() {
        return this.inputManager.getMousePos();
    }

    public void sendKeys(String string, int keywait, int keymodwait) {
        this.inputManager.sendKeys(string, keywait, keymodwait);
    }

    public void holdKey(int keycode) {
        this.inputManager.holdKey(keycode);
    }

    public boolean isKeyDown(int keycode) {
        return this.inputManager.isKeyDown(keycode);
    }

    public void releaseKey(int keycode) {
        this.inputManager.releaseKey(keycode);
    }

    public void setGraphics(boolean On) {
        renderWhileBlocking = On;
        Main.getFrame().updateUI();
    }

    public void setDebug(boolean on) {
        debugfx = on;
        Main.getFrame().updateUI();
    }

    public boolean isMouseEnabled() {
        return this.inputManager.isMouseInputEnabled();
    }

    public boolean isKeyboardEnabled() {
        return this.inputManager.isKeyInputEnabled();
    }

    public void setMouseInput(boolean Enabled) {
        this.inputManager.setMouseInputEnabled(Enabled);
        Main.getFrame().updateUI();
    }

    public void setKeyInput(boolean Enabled) {
        this.inputManager.setKeyInputEnabled(Enabled);
        Main.getFrame().updateUI();
    }
    
    public Graphics drawGraphics(Graphics2D G) throws NoSuchFieldException, IllegalArgumentException, IllegalAccessException {
        if (active) {
            switch (Mode) {
                case SafeMode:
                    Field rasterField = BufferedImage.class.getDeclaredField("raster");
                    rasterField.setAccessible(true);
                    WritableRaster bufferRaster = (WritableRaster) rasterField.get(this.gameImage);
                    final int[] bufferData = ((DataBufferInt) bufferRaster.getDataBuffer()).getData();
                    WritableRaster debugRaster = (WritableRaster) rasterField.get(this.debugImage);
                    final int[] debugData = ((DataBufferInt) debugRaster.getDataBuffer()).getData();
                    final Graphics debugGraphics = this.debugImage.getGraphics();
                    debugGraphics.setColor(Color.green);

                    int Length = this.gameImage.getHeight() * this.gameImage.getWidth();


                    gameBuffer.rewind();
                    gameBuffer.put(bufferData, 0, Length);
                    if (renderWhileBlocking && !Main.getFrame().isMinimized()) {
                        final Point P = getMousePos();
                        if (debugfx) {
                            debugBuffer.rewind();
                            for (int I = 0; I < Length; ++I) {
                                int color = debugBuffer.get();
                                if (color != transColor) {
                                    debugData[I] = color;
                                } else {
                                    debugData[I] = bufferData[I];
                                }
                            }
                        } else {
                            debugGraphics.drawImage(this.gameImage, 0, 0, null);
                        }

                        //Draw Mouse...
                        if (this.drawMouse) {
                            debugGraphics.fillOval(P.x - 2, P.y - 2, 4, 4);
                        }

                        if (G != null) {
                            G.drawImage(this.debugImage, 0, 0, null);
                        }
                    }
                    break;

                case OpenGL:
                    if (GLDXLoader.OpenGLLoaded) {
                        GLDXLoader.GetOpenGLStaticBuffer(gameBuffer);
                    }
                    break;

                case DirectX:
                    if (GLDXLoader.DirectXLoaded) {
                        GLDXLoader.GetDirectXStaticBuffer(gameBuffer);
                    }
                    break;

                default:
                    return G;
            }

            return this.gameImage.getGraphics();
        }
        return null;
    }
}
