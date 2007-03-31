package edu.cmu.sphinx.tools.dictator;

import com.intellij.uiDesigner.core.GridConstraints;
import com.intellij.uiDesigner.core.GridLayoutManager;
import edu.cmu.sphinx.frontend.util.StreamDataSource;
import edu.cmu.sphinx.frontend.util.VUMeterPanel;
import edu.cmu.sphinx.recognizer.Recognizer;
import edu.cmu.sphinx.result.Result;
import edu.cmu.sphinx.tools.audio.Recorder;
import edu.cmu.sphinx.tools.batch.BatchNISTRecognizer;
import edu.cmu.sphinx.tools.corpus.RegionOfAudioData;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.*;
import java.lang.reflect.Method;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

/**
 * Copyright 1999-2006 Carnegie Mellon University.
 * Portions Copyright 2001 David Wallace Croft
 * All Rights Reserved.  Use is subject to license terms.
 * <p/>
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 * <p/>
 * User: Peter Wolf
 * Date: Apr 30, 2006
 * Time: 11:38:42 AM
 */
public class DictatorView {
    private JButton clearText;
    private VUMeterPanel vu;
    private Recorder recorder;
    JPanel mainPanel;
    ClipboardControls clipboard;
    private JTextArea text;
    Recognizer recognizer;
    StreamDataSource dataSource;
    private JButton donateVoice;
    //Font font;

    public void create() throws Exception {

        clipboard = new ClipboardControls();

        text.addMouseListener(
                new MouseListener() {

                    public void mouseClicked(MouseEvent event) {

                    }

                    public void mousePressed(MouseEvent event) {
                        //text.insert( vu.getVu().getRmsDB() + " ", text.getSelectionStart() );
                        clipboard.setClipboardContents(text.getText());
                        String utt = "utt" + System.currentTimeMillis();
                        try {
                            recorder.start(utt);
                        } catch (IOException e) {
                            throw new Error(e);
                        }
                    }

                    public void mouseReleased(MouseEvent event) {
                        new DictatorThread().start();

                    }

                    public void mouseEntered(MouseEvent event) {
                    }

                    public void mouseExited(MouseEvent event) {
                    }
                }
        );

        clearText.addActionListener(
                new ActionListener() {
                    public void actionPerformed(ActionEvent event) {
                        text.setText("");
                    }
                }
        );

        donateVoice.addActionListener(
                new ActionListener() {
                    public void actionPerformed(ActionEvent event) {
                        try {
                            showDocument(new URL("http://mangueira.speech.cs.cmu.edu:8080/data-collection/jsps/login.jsp"));
                        } catch (MalformedURLException e) {
                            throw new Error(e);
                        }
                    }
                }
        );

    }

    /**
     * ******************************************************************
     * showDocument(), getBasicServiceObject() and getBasicServiceClass()
     * are adapteted from a JNLP tutorial by David Croft.
     *
     * *******************************************************************
     * @see <a target="_blank"
     *      href="http://croftsoft.com/library/tutorials/browser/">
     *      Launching a Browser from Java</a>
     * @since 2001-08-31
     */

    public static boolean showDocument(URL url) {

        Object basicServiceObject
                = getBasicServiceObject();

        Class basicServiceClass
                = getBasicServiceClass();

        if (basicServiceObject == null) {
            return false;
        }

        try {
            Method method = basicServiceClass.getMethod(
                    "showDocument", new Class [ ]{URL.class});

            Boolean resultBoolean = (Boolean)
                    method.invoke(basicServiceObject, new Object [ ]{url});

            return resultBoolean.booleanValue();
        }
        catch (Exception ex) {
            ex.printStackTrace();

            throw new RuntimeException(ex.getMessage());
        }
    }

    private static Object getBasicServiceObject() {
        try {
            Class serviceManagerClass
                    = Class.forName("javax.jnlp.ServiceManager");

            Method lookupMethod = serviceManagerClass.getMethod("lookup",
                    new Class [ ]{String.class});

            return lookupMethod.invoke(
                    null, new Object [ ]{"javax.jnlp.BasicService"});
        }
        catch (Exception ex) {
            return null;
        }
    }

    private static Class getBasicServiceClass() {
        try {
            return Class.forName("javax.jnlp.BasicService");
        }
        catch (Exception ex) {
            return null;
        }
    }

    private void load(String configFile) throws IOException, PropertyException, InstantiationException, URISyntaxException {
        text.setText("Unpacking Language Model...");

        unpackLM();

        text.setText("Loading Recognizer...");

        loadRecognizer(configFile);

        vu.setVu(recorder.getVUMeter());
        vu.start();
        //font = Font.createFont(Font.TRUETYPE_FONT, new FileInputStream(fontFile)).deriveFont(20.0f);
        //text.setFont(font);
        text.setText("");
    }

    private void loadRecognizer(String configFile) throws IOException, PropertyException, InstantiationException {
        URL url = new File(configFile).toURI().toURL();
        ConfigurationManager cm = new ConfigurationManager(url);
        recognizer = (Recognizer) cm.lookup("wordRecognizer");
        recognizer.allocate();
        dataSource = (StreamDataSource) cm.lookup("streamDataSource");
        recorder = new Recorder();
        recorder.open(cm);
    }


    private void unpackLM() throws IOException, URISyntaxException {
        File dmpFile = new File("wsj5k.DMP");

        if (!dmpFile.canRead()) {

            InputStream in = getClass().getResourceAsStream(new URI("resource:/wsj5k.DMP").getPath());
            OutputStream out = new FileOutputStream(dmpFile);

            // Transfer bytes from in to out
            byte[] buf = new byte[1024];
            int len;
            while ((len = in.read(buf)) > 0) {
                out.write(buf, 0, len);
            }
            in.close();
            out.close();
        }
    }

    public static String hex2Unicode(String hex) {
        if (hex.startsWith("<")) return hex;
        byte[] bytes = BatchNISTRecognizer.hex2Binary(hex);
        try {
            return new String(bytes, "GB2312");
        } catch (UnsupportedEncodingException e) {
            throw new Error(e);
        }
    }

    {
// GUI initializer generated by IntelliJ IDEA GUI Designer
// >>> IMPORTANT!! <<<
// DO NOT EDIT OR ADD ANY CODE HERE!
        $$$setupUI$$$();
    }

    /**
     * Method generated by IntelliJ IDEA GUI Designer
     * >>> IMPORTANT!! <<<
     * DO NOT edit this method OR call it in your code!
     */
    private void $$$setupUI$$$() {
        mainPanel = new JPanel();
        mainPanel.setLayout(new GridLayoutManager(2, 1, new Insets(0, 0, 0, 0), -1, -1));
        final JPanel panel1 = new JPanel();
        panel1.setLayout(new GridLayoutManager(1, 2, new Insets(0, 0, 0, 0), -1, -1));
        mainPanel.add(panel1, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, null, null));
        final JScrollPane scrollPane1 = new JScrollPane();
        panel1.add(scrollPane1, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_WANT_GROW, new Dimension(500, 300), null, null));
        text = new JTextArea();
        scrollPane1.setViewportView(text);
        final JPanel panel2 = new JPanel();
        panel2.setLayout(new GridLayoutManager(1, 1, new Insets(0, 0, 0, 0), -1, -1));
        panel1.add(panel2, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, null, null));
        vu = new VUMeterPanel();
        panel2.add(vu, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, new Dimension(-1, 300), null, null));
        final JPanel panel3 = new JPanel();
        panel3.setLayout(new GridLayoutManager(1, 2, new Insets(0, 0, 0, 0), -1, -1));
        mainPanel.add(panel3, new GridConstraints(1, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, null, null));
        clearText = new JButton();
        clearText.setText("Clear Text");
        panel3.add(clearText, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null));
        donateVoice = new JButton();
        donateVoice.setText("Donate Voice");
        panel3.add(donateVoice, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null));
    }

    class DictatorThread extends Thread {

        public void run() {
            try {
                RegionOfAudioData rad = recorder.stop();
                dataSource.setInputStream(rad.toInputStream(), "Dictator");
                Result result = recognizer.recognize();
                String res = result.getBestResultNoFiller();
                /*
                for( String word : res.split(" ") ) {
                    text.append(hex2Unicode(word));
                }*/
                text.append(res + " ");
            } catch (Throwable e) {
                System.out.println(e);
            }
        }
    }

    public static void main(String[] args) {
        /*
        if (args.length != 2) {
            System.out.println(
                    "Usage: DictatorView propertiesFile fontFile");
            System.exit(1);
        }
        */
        try {

            final DictatorView dv = new DictatorView();
            JFrame f = new JFrame("Eval Dictator");

            f.setContentPane(dv.mainPanel);
            f.pack();
            f.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);

            dv.create();

            f.setVisible(true);

            dv.load("dictator.config.xml");

            System.out.println("Ready");

        } catch (Exception e) {
            throw new Error(e);
        }
    }
}
