package JNI;

import java.awt.geom.AffineTransform;
import java.awt.image.AffineTransformOp;
import java.awt.image.BufferedImage;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;
import smart.OperatingMode;

public class GLDXLoader {

    private int ByteSize = 0;
    private IntBuffer IBuffer = null;
    private ByteBuffer BBuffer = null;
    private OperatingMode Mode = null;
    private BufferedImage Image = null;
    public static boolean OpenGLLoaded = false;
    public static boolean DirectXLoaded = false;
    //ByteSize = ((ImageWidth * BitsPerPixel + 31) / 32) * 4 * ImageHeight;

    public static native void GetOpenGLBuffer(IntBuffer Buffer); //Needs to be ran in a thread (Smart 8.0/8.1 does this).
    public static native void GetOpenGLStaticBuffer(IntBuffer Buffer); //Called once and the plugin stores the pointer and writes to it every frame.

    public static native void GetDirectXBuffer(IntBuffer Buffer);
    public static native void GetDirectXStaticBuffer(IntBuffer Buffer);
    
    public static native void DrawMouse(int X, int Y, int R, int G, int B);

    public GLDXLoader(ByteBuffer Buffer, int ImageWidth, int ImageHeight) {
        if (Buffer != null) {
            this.BBuffer = Buffer;
            this.IBuffer = BBuffer.order(ByteOrder.LITTLE_ENDIAN).asIntBuffer();
        }
    }

    public GLDXLoader(int ImageWidth, int ImageHeight) {
        ByteSize = ImageWidth * ImageHeight * 4;
        BBuffer = ByteBuffer.allocateDirect(ByteSize).order(ByteOrder.LITTLE_ENDIAN);
        IBuffer = BBuffer.asIntBuffer();
    }

    public void SetSize(int ImageWidth, int ImageHeight) {
        BBuffer = null;
        IBuffer = null;
        ByteSize = ImageWidth * ImageHeight * 4;
        BBuffer = ByteBuffer.allocateDirect(ByteSize).order(ByteOrder.LITTLE_ENDIAN);
        IBuffer = BBuffer.asIntBuffer();
    }

    private void readBuffer(IntBuffer Buffer, OperatingMode Mode) {
        Buffer.rewind();
        switch (Mode) {
            case OpenGL:
                GetOpenGLBuffer(Buffer);
            case DirectX:
                GetDirectXBuffer(Buffer);
            default:
                break;
        }
    }

    public void setMode(OperatingMode Mode) {
        this.Mode = Mode;
    }

    public ByteBuffer GetBuffer() {
        readBuffer(IBuffer, Mode);
        return BBuffer;
    }

    public void GetBuffer(ByteBuffer Buffer) {
        readBuffer(Buffer.asIntBuffer(), Mode);
    }

    public void GetBuffer(IntBuffer Buffer) {
        readBuffer(Buffer, Mode);
    }

    public BufferedImage VerticalFlip(BufferedImage imageIn) {
        AffineTransform tx = AffineTransform.getScaleInstance(1, -1);
        tx.translate(0, -Image.getHeight(null));
        AffineTransformOp op = new AffineTransformOp(tx, AffineTransformOp.TYPE_NEAREST_NEIGHBOR);
        imageIn = op.filter(imageIn, null);
        return imageIn;
    }

    public BufferedImage HorizontalFlip(BufferedImage imageIn) {
        AffineTransform tx = AffineTransform.getScaleInstance(-1, 1);
        tx.translate(-Image.getHeight(null), 0);
        AffineTransformOp op = new AffineTransformOp(tx, AffineTransformOp.TYPE_NEAREST_NEIGHBOR);
        imageIn = op.filter(imageIn, null);
        return imageIn;
    }

    public BufferedImage AxisFlip(BufferedImage imageIn) {
        AffineTransform tx = AffineTransform.getScaleInstance(-1, -1);
        tx.translate(-Image.getWidth(null), -Image.getWidth(null));
        AffineTransformOp op = new AffineTransformOp(tx, AffineTransformOp.TYPE_NEAREST_NEIGHBOR);
        imageIn = op.filter(imageIn, null);
        return imageIn;
    }
}