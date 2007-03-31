package edu.cmu.sphinx.tools.datacollection.client;

import edu.cmu.sphinx.frontend.util.VUMeter;
import edu.cmu.sphinx.tools.audio.Player;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;
import netscape.javascript.JSObject;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;


/**
 * Created by IntelliJ IDEA.
 * User: bertrand
 * Date: Apr 10, 2006
 * Time: 11:56:53 PM
 * Copyright Apr 10, 2006, All rights reserved.
 * AudioControllerApplet is the desktop internet browser specific presentation logic for AudioController
 * It is responsible for directing and interacting with the subject through a successfull data collection
 * session.
 */
public class  AudioCollectorPresentationApplet extends JApplet
        implements KeyListener, ActionListener {

    AudioCollector _audioCollector = null;
    ConfigurationManager _audioConfigManager = null;

    //Below are the applet's widgets
    JButton _previousButton = null;
    JButton _recordButton = null;
    JButton _playButton = null;
    JButton _stopButton = null;
    JButton _nextButton = null;
    JButton _submitButton = null;

    //Below are browser DOM Objects we will be manipulating
    JSObject _billboard = null;
    JSObject _eqAverageNormal = null;
    JSObject _eqAverageMid = null;
    JSObject _eqAverageHi = null;
    JSObject _eqMax = null;
    JSObject _eqPeak = null;
    JSObject _eqRMS = null;

    static final byte BAR_LENGTH = 25;
    static String BAR = "";
    URL _submitURL;
    String[] BARS = new String[BAR_LENGTH];
    JRootPane _content = null;
    Player _currentPlayer = null;
    JButton _defaultButton = _nextButton;

    /**
     * Convenience method sets the passed button as the default button
     * @param b
     */
    private void setDefaultButton(JButton b) {
       _defaultButton = b;
       _defaultButton.grabFocus();
    }

    public void keyPressed(KeyEvent e) {

    }

    /**
     * Implements the keyboard shortkey, if anybutton is pressed the current default button will be "clicked"
     * @param e
     */
    public void keyTyped(KeyEvent e) {
        System.out.println("KeyPressed: action="+_defaultButton.getName());

       _defaultButton.doClick();
    }

    public void keyReleased(KeyEvent e) {
    }

    /**
     * upon any mouse clicks, translates the button that is currently being chosen by the user
     * into actions for the AudioController, keep in mind that buttons that should not be clicked
     * based on the current AudioController's state, are disabled so we assume here that only legal buttons
     * are clicked. (no checking for legality).
     * @param e
     */
    public void actionPerformed(ActionEvent e) {
        if (e.getSource() == _previousButton) {
            setDefaultButton(_playButton);
            _audioCollector.previous();

        }
        else if (e.getSource() == _recordButton) {
            setDefaultButton(_stopButton);
            _audioCollector.record();

        }
        else if (e.getSource() == _playButton) {
            _currentPlayer = _audioCollector.play();
            setDefaultButton(_stopButton);
            _currentPlayer.start();

            //Starts a thread to Auto Stop at the end of the recording w/o user interacton
            new Thread() {
                    public void run() {
                        while (_currentPlayer.isPlaying()) {
                            try {
                                sleep(1000);
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }
                      if (_audioCollector.isStopActive()) _stopButton.doClick();
                    }
                }.start();
        }

        else if (e.getSource() == _stopButton) {
            if (_currentPlayer != null && _currentPlayer.isPlaying()) _currentPlayer.stop();

            setDefaultButton(_nextButton);
            _audioCollector.stop();

        }
        else if (e.getSource() == _nextButton) {
            setDefaultButton(_recordButton);
            _audioCollector.next();

        }
         else if (e.getSource() == _submitButton) {
            setDefaultButton(_submitButton);
            _billboard.setMember("value", "Sending data collected...");
            _audioCollector.submit(_submitURL);
            _billboard.setMember("value", "All done! Thank YOU for participating in this data collection effort.");

        }
        refresh();
    }

    /**
     * returns a progress bar based on the parameters passed
     * @param metric1  parameter to map
     * @param max     maximum value that parameter may have
     * @return  a progress bar in the format of a string
     */
    String displayBar(final double metric1,  final double max) {
        byte b = (byte) (BAR_LENGTH * (metric1 / max));

        if (BARS[b] == null) {

            StringBuffer sbuf = new StringBuffer(BAR);
            for (byte i = 0; i < b; i++) {
                sbuf.setCharAt(i, '\u2588');
            }
            BARS[b] = sbuf.toString();
        }
        return BARS[b];
    }

    /**
     * Marks the progress bar with local maximum for effect
     * @param mark
     * @param max
     * @return progress bar as a String
     */
    private String maxBar(double mark, double max){
        StringBuffer retBuf = new StringBuffer(BAR);
        byte b2 = (byte) (BAR_LENGTH * (mark / max));
        retBuf.setCharAt(b2, '\u25BA');
        return retBuf.toString();
    }

    /**
     * starts a thread to constantly display the ambient volume (or playback) associate with the
     * AudioCollector
     */
    void displayEQ() {

        Thread eq = new Thread() {

            public void run() {
                double maxNormal = 0.0;
                double maxHi = 0.0;
                double maxMid = 0.0;
                final double MID_CUTOFF = 26.0;
                String oldStatus = "";

                System.out.println("Entering while loop...");
                while (true) {
                    final VUMeter vum = _audioCollector.getCurrentVUMeter();
                    final double average = vum.getAverageDB() ;
                    final double peak = vum.getPeakDB();
                    final double hi = vum.getMaxDB();
                    final String bar = displayBar(average, hi);

                    String status;

                    //displays the red clipping bar
                    if (vum.getIsClipping()) {
                        if (maxHi < peak) maxHi = peak;
                        _eqAverageHi.setMember("value", bar);
                        _eqAverageMid.setMember("value", maxBar(maxMid, hi));
                        _eqAverageNormal.setMember("value", maxBar(maxNormal, hi));
                        status = "volume: Clipping";
                    }
                    //displays the orange bar if close to clipping
                    else if (vum.getMaxDB() - vum.getAverageDB() < MID_CUTOFF) {
                        if (maxMid < peak) maxMid = peak;
                        _eqAverageMid.setMember("value", bar);
                        _eqAverageNormal.setMember("value", maxBar(maxNormal, hi));
                        _eqAverageHi.setMember("value", maxBar(maxHi, hi));
                        status = "volume: Warning";
                    }
                     //Displays the green bar if audio levels are normal
                    else {
                        if (maxNormal < peak) maxNormal = peak;
                        _eqAverageNormal.setMember("value", bar);
                        _eqAverageMid.setMember("value", maxBar(maxMid, hi));
                        _eqAverageHi.setMember("value", maxBar(maxHi, hi));
                        status = "volume: Normal";
                    }
                    if (!oldStatus.equals(status)) System.out.println(oldStatus=status);
                    try {
                        sleep(40);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }

            }
        };
        eq.setDaemon(true);
        eq.start();
    }

    /**
     * refreshes all the widgets on the JApplet based on the current state of the AudioCollector
     */
    void refresh() {
        _previousButton.setEnabled(_audioCollector.isPreviousActive());
        _recordButton.setEnabled(_audioCollector.isRecordActive());
        _playButton.setEnabled(_audioCollector.isPlayActive());
        _stopButton.setEnabled(_audioCollector.isStopActive());
        _nextButton.setEnabled(_audioCollector.isNextActive());
        _submitButton.setVisible(_audioCollector.isSubmitActive());
        _nextButton.setVisible(!_audioCollector.isSubmitActive());
        if (_audioCollector.getCurrentState() != AudioCollector.INVALID_STATE)
            _billboard.setMember("value", getTranscript());
    }

    public String getTranscript() {
        return _audioCollector.getCurrentTranscript();
    }

      private URL getResourceURL(String resource) {
       return AudioCollectorPresentationApplet.class.getResource(resource);
    }

    /**
     * Interacts with the JSP to get all the information necessary to create an AudioCollector for this session
     * and creates all the GUI Objects necessary (include the JSObjects so we can change the Browser's DOM on the
     * fly a la AJAX). parameter from JSP: "corpus" : serialized corpus (using CorpusTranscriptsOnlyWriter, "store"
     * the url to a jsp that will received the resulting zip stream, "config" a url to a config Manager file specifying
     * the specifics of the client native audio capabilities.
     */
    public void init() {
        String corpusAsString = getParameter("corpus");

        URL url;
        String submitURLString;
        try {
            url = new URL(getParameter("config"));
            submitURLString = getParameter("store");
            _audioConfigManager = new ConfigurationManager(url);
            _audioCollector = new AudioCollector(corpusAsString, _audioConfigManager);

            _submitURL = new URL(submitURLString + "?ID=" + _audioCollector._corpus.getProperty("ID") +
                    "&userID=" + _audioCollector._corpus.getProperty("userID"));
        } catch (MalformedURLException e) {
            e.printStackTrace();
        } catch (PropertyException e) {
            e.printStackTrace();

        } catch (IOException e) {
            e.printStackTrace();
        }
        _previousButton = new JButton(new ImageIcon(getResourceURL("images/previous.gif")));
        _recordButton = new JButton(new ImageIcon(getResourceURL("images/record.gif")));
        _playButton = new JButton(new ImageIcon(getResourceURL("images/play.gif")));
        _stopButton = new JButton(new ImageIcon(getResourceURL("images/stop.gif")));
        _nextButton = new JButton(new ImageIcon(getResourceURL("images/next.gif")));
        _submitButton = new JButton("submit");

        _content = getRootPane();
        _content.setLayout(new FlowLayout());
        _content.setBackground(Color.gray);

        _content.add(_previousButton);
        _content.add(_recordButton);
        _content.add(_playButton);
        _content.add(_stopButton);
        _content.add(_nextButton);
        _content.add(_submitButton);

        _previousButton.setName("previous button");
        _recordButton.setName("record button");
        _playButton.setName("play button");
        _stopButton.setName("stop button");
        _nextButton.setName("next button");
        _submitButton.setName("submit button");

        _previousButton.addActionListener(this);
        _recordButton.addActionListener(this);
        _playButton.addActionListener(this);
        _stopButton.addActionListener(this);
        _nextButton.addActionListener(this);
        _submitButton.addActionListener(this);

        _previousButton.addKeyListener(this);
        _recordButton.addKeyListener(this);
        _playButton.addKeyListener(this);
        _stopButton.addKeyListener(this);
        _nextButton.addKeyListener(this);
        _submitButton.addKeyListener(this);
        _content.addKeyListener(this);

        setDefaultButton(_recordButton);

        JSObject MainWin = JSObject.getWindow(this);
        JSObject DocumentPage = (JSObject) MainWin.getMember("document");
        JSObject billboardForm = (JSObject) DocumentPage.getMember("billboard");
        JSObject eqForm = (JSObject) DocumentPage.getMember("eq");
        _billboard = (JSObject) billboardForm.getMember("transcript");
        _eqAverageNormal = (JSObject) eqForm.getMember("average-normal");
        _eqAverageMid = (JSObject) eqForm.getMember("average-mid");
        _eqAverageHi = (JSObject) eqForm.getMember("average-hi");

        for (int i=0; i<BAR_LENGTH; i++) {
            BAR +='\u0020';
        }

        displayEQ();
        refresh();
    }

    /**
     * calls AudioCollector's destroy for cleanup
     */
    public void destroy() {
        super.destroy();
        _audioCollector.destroy();
    }
    public String toString() {
        return "record=" + _audioCollector.isRecordActive() +
                ",play=" + _audioCollector.isPlayActive() +
                ",stop=" + _audioCollector.isStopActive();
    }
}
