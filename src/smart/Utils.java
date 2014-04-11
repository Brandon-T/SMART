package smart;

import java.awt.Image;
import java.awt.image.*;
import javax.imageio.ImageIO;
import javax.swing.*;

public class Utils {

    private static Utils instance = new Utils();

    private Utils() {
    }

    public static Utils getInstance() {
        return instance;
    }

    public static void sleep(int Time) {
        try {
            Thread.sleep(Time);
        } catch (Exception Ex) {
            Main.StackTrace(Ex);
        }
    }

    public static void setCustomTheme(String Theme) {
        try {
            UIManager.setLookAndFeel(Theme);
        } catch (ClassNotFoundException | InstantiationException | IllegalAccessException | UnsupportedLookAndFeelException Ex) {
            Main.StackTrace(Ex);
        }
    }

    public Image scaleImage(int width, int height, Image Img) {
        return Img.getScaledInstance(width, height, Image.SCALE_SMOOTH);
    }

    public BufferedImage loadResourceImage(String ResourcePath) {
        try {
            return ImageIO.read(getClass().getResource(ResourcePath));
        } catch (Exception Ex) {
            Main.StackTrace(Ex);
        }
        return null;
    }

    public ImageIcon loadResourceIcon(String ResourcePath) {
        return new ImageIcon(getClass().getResource(ResourcePath));
    }

    public Image grayScale(Image Img, int GrayPercentage) {
        ImageProducer Producer = new FilteredImageSource(Img.getSource(), new GrayFilter(true, GrayPercentage));
        return java.awt.Toolkit.getDefaultToolkit().createImage(Producer);
    }
}
