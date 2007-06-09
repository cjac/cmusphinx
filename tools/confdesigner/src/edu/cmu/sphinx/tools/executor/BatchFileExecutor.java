package edu.cmu.sphinx.tools.executor;

import edu.cmu.sphinx.recognizer.Recognizer;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.S4Component;
import edu.cmu.sphinx.util.props.S4String;
import edu.cmu.sphinx.frontend.util.AudioFileDataSource;
import edu.cmu.sphinx.result.Result;

import java.io.File;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.List;
import java.util.ArrayList;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class BatchFileExecutor implements Executable {

    @S4Component(type = Recognizer.class)
    public static final String PROP_RECOGNIZER = "recognizer";

    @S4Component(type = AudioFileDataSource.class)
    public static final String PROP_DATA_SOURCE = "dataSource";

    @S4String
    public static final String PROP_DRIVER = "batchFile";

    private Recognizer recognizer;
    private String batchFile;
    private AudioFileDataSource dataSource;
    private String name;


    public void doExecute() {
        List<File> files = readDriver(batchFile);
        for (File file : files) {
            dataSource.setAudioFile(file, file.getPath());

            Result r;
            while ((r = recognizer.recognize()) != null) {
                System.out.print(r.getBestResultNoFiller() + "");
            }

            System.out.println("");
        }

    }


    public void newProperties(PropertySheet ps) throws PropertyException {
        dataSource = (AudioFileDataSource) ps.getComponent(PROP_DATA_SOURCE);
        recognizer = (Recognizer) ps.getComponent(PROP_RECOGNIZER);
        batchFile = ps.getString(PROP_DRIVER);
        name = ps.getInstanceName();

        assert batchFile != null : "batch file is not set!";
    }


    public String getName() {
        return null;
    }

        /** Reads and verifies a driver file. */
    public static List<File> readDriver(String fileName) {
        File inputFile = new File(fileName);
        List<File> driverFiles = null;

        try {
            if (!inputFile.isFile() || !inputFile.canRead())
                throw new IllegalArgumentException("file to read is not valid");

            BufferedReader bf = new BufferedReader(new FileReader(inputFile));

            driverFiles = new ArrayList<File>();

            String line;
            while ((line = bf.readLine()) != null && line.trim().length() != 0) {
                File file = new File(line);

                assert file.isFile() : "file " + file + " does not exist!";
                driverFiles.add(file);
            }

            bf.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        assert driverFiles != null;
        return driverFiles;

    }
}
