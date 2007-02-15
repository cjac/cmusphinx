namespace edu.cmu.sphinx.riddler
{
	using System;
	using NUnit.Framework;   

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
		[Test] public void CreateDictionaryAndCorpus() 
		{
            dictionaryDescriptor did = new dictionaryDescriptor();
            dictionaryDescriptorEntry dictMetadatum = new dictionaryDescriptorEntry();
            dictMetadatum.key = "SomeKey";
            dictMetadatum.value = "SomeValue";
            did.metadata = new dictionaryDescriptorEntry[] { dictMetadatum };
            dictionaryID dictID = riddler.createDictionary(did);
            
            Assert.AreEqual(dictID, riddler.getDictionary(did));

            corpusDescriptor cod = new corpusDescriptor();
            cod.collectDate = DateTime.Today;
            corpusDescriptorEntry corpMetadatum = new corpusDescriptorEntry();
            corpMetadatum.key = "SomeKey";
            corpMetadatum.value = "SomeValue";
            cod.metadata = new corpusDescriptorEntry[] { corpMetadatum };
            corpusID corpID = riddler.createCorpus(dictID, cod);

            corpusDescriptor fetched = riddler.getCorpusDescriptor(corpID);
            Assert.AreEqual(cod, fetched);
            Assert.AreEqual(cod.metadata, fetched.metadata);
            Assert.AreEqual(cod.collectDate, fetched.collectDate);
		}

		[Test]
        [ExpectedException(typeof(InvalidCastException))]
		public void ExpectAnException()
		{
			throw new InvalidCastException();
		}

		[Test]
		[Ignore("ignored test")]
		public void IgnoredTest()
		{
			throw new Exception();
		}
	}
}