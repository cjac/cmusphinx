package edu.cmu.sphinx.tools.dictator;

import edu.cmu.sphinx.frontend.util.StreamDataSource;
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
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.URL;

import com.intellij.uiDesigner.core.GridLayoutManager;
import com.intellij.uiDesigner.core.GridConstraints;

/**
 * Copyright 1999-2006 Carnegie Mellon University.
 * Portions Copyright 2002 Sun Microsystems, Inc.
 * Portions Copyright 2002 Mitsubishi Electric Research Laboratories.
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
    private JButton clear;
    private JButton undo;
    private VUMeterPanel vu;
    private Recorder recorder;
    JPanel mainPanel;
    ClipboardControls clipboard;
    private JTextArea text;
    Recognizer recognizer;
    StreamDataSource dataSource;
    Font font;

    public void open(String configFile, String fontFile) throws Exception {

        clipboard = new ClipboardControls();

        text.addMouseListener(
                new MouseListener() {

                    public void mouseClicked(MouseEvent event) {

                    }

                    public void mousePressed(MouseEvent event) {
                        //text.insert( vu.getVu().getRmsDB() + " ", text.getSelectionStart() );
                        clipboard.setClipboardContents( text.getText());
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

        clear.addActionListener(
                new ActionListener() {
                    public void actionPerformed(ActionEvent event) {
                        text.setText("");
                    }
                }
                );


        text.setText("Loading...");



        loadRecognizer(configFile);

        vu.setVu(recorder.getVUMeter());
        vu.start();
        font = Font.createFont(Font.TRUETYPE_FONT, new FileInputStream(fontFile)).deriveFont(20.0f);
        text.setFont(font);
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

    public static String hex2Unicode(String hex) {
        if (hex.startsWith("<")) return hex;
        byte[] bytes = BatchNISTRecognizer.hex2Binary(hex);
        try {
            return new String(bytes, "GB2312");
        } catch (UnsupportedEncodingException e) {
            throw new Error(e);
        }
    }

    class DictatorThread extends Thread {

        public void run() {
                        try {
                            RegionOfAudioData rad = recorder.stop();
                            dataSource.setInputStream( rad.toInputStream(), "Dictator" );
                            Result result = recognizer.recognize();
                            String res = result.getBestResultNoFiller();
                            /*
                            for( String word : res.split(" ") ) {
                                text.append(hex2Unicode(word));
                            }*/
                            text.append(res+" ");
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
            JFrame f = new JFrame("Haikya Dictator");

            f.setContentPane(dv.mainPanel);
            f.pack();
            f.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);

            dv.open("dictator.config.xml","mssong.ttf");

            f.setVisible(true);

            System.out.println("Ready");

        } catch (Exception e) {
            throw new Error(e);
        }
    }
}
