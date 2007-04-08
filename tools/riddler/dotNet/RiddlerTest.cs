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
		[Test] public void CreateDictionary() 
		{
            metadataWrapper mdw = new metadataWrapper();
            metadataWrapperEntry datum = new metadataWrapperEntry();
            datum.key = "SomeKey";
            datum.value = "SomeValue";
            mdw.contents = new metadataWrapperEntry[] { datum };

            string dictId = riddler.createDictionary(mdw);
            string retrieved = riddler.getDictionary(mdw);
            Assert.AreEqual(dictId, retrieved);
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