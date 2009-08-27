package edu.cmu.sphinx.util.props;


import java.io.IOException;
public interface SetId extends Configurable {
    public void setId (String base, String id)  throws IOException;
    public void setId (String id)  throws IOException;
}
