namespace edu.cmu.sphinx.riddler
{
	using System;
	using NUnit.Framework;
    using System.Collections.Generic;

    internal class EntryComparer : IEqualityComparer<metadataWrapperEntry>
    {
        #region IEqualityComparer<metadataWrapperEntry> Members

        public bool Equals(metadataWrapperEntry x, metadataWrapperEntry y)
        {
            return x.key.Equals(y.key) && x.value.Equals(y.value);
        }

        public int GetHashCode(metadataWrapperEntry obj)
        {
            return obj.key.GetHashCode() * obj.value.GetHashCode();
        }

        #endregion
    }

	/// <summary>NUnit test suite for Sphinx Riddler web services</summary>
	/// 
	[TestFixture] 
	public class RiddlerTest
	{
        RiddlerBeanService riddler;

		/// <summary>
		/// 
		/// </summary>
		[SetUp] public void Init() 
		{
            riddler = new RiddlerBeanService();
		}

		/// <summary>
		/// 
		/// </summary>
		///
		[Test] public void CreateAndFetchDictionaryAndMetadata() 
		{
            metadataWrapper mdw = new metadataWrapper();
            metadataWrapperEntry datum = new metadataWrapperEntry();
            datum.key = "SomeKey";
            datum.value = "SomeValue";
            metadataWrapperEntry datum2 = new metadataWrapperEntry();
            datum2.key = "AnotherKey";
            datum2.value = "AnotherValue";
            mdw.contents = new metadataWrapperEntry[] { datum, datum2 };

            string dictId = riddler.createDictionary(mdw);
            string retrieved = riddler.getDictionary(mdw);
            Assert.AreEqual(dictId, retrieved);

            metadataWrapper mdwRetrieved = riddler.getDictionaryMetadata(dictId);
            
            // annoyance-- have to use an IDictionary because we need to specify an equality comparator
            // (you don't get one for free from the stub impl of metadataWrapperEntry)
            IDictionary<metadataWrapperEntry, int> tmpDict = new Dictionary<metadataWrapperEntry, int>(new EntryComparer());
            foreach (metadataWrapperEntry e in mdwRetrieved.contents)
                tmpDict.Add(new KeyValuePair<metadataWrapperEntry, int>(e, 42));
            
            Assert.IsTrue(tmpDict.Contains(new KeyValuePair<metadataWrapperEntry, int>(datum, 42)));
            Assert.IsTrue(tmpDict.Contains(new KeyValuePair<metadataWrapperEntry, int>(datum2, 42)));
		}

		[Test]
        [ExpectedException(typeof(System.Web.Services.Protocols.SoapException))]
		public void FetchNonexistentDictionary()
		{
            metadataWrapper mdw = new metadataWrapper();
            metadataWrapperEntry datum = new metadataWrapperEntry();
            datum.key = "KeyX";
            datum.value = "ValueY";
            mdw.contents = new metadataWrapperEntry[] { datum };
            
            riddler.getDictionary(mdw);			
		}

        [Test]
        [ExpectedException(typeof(System.Web.Services.Protocols.SoapException))]
        public void FetchNonexistentDictionaryMetadata()
        {
            riddler.getDictionaryMetadata("nonexistent-dictionary-ID");
        }

        [Test]
		[Ignore("ignored test")]
		public void IgnoredTest()
		{
			throw new Exception();
		}
	}
}