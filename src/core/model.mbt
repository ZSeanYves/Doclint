pub(all) struct DocMeta {
  title : String
  author : String
  created_at : String
}

pub(all) struct Page {
  index : Int
  text  : String
}

pub(all) struct Document {
  meta  : DocMeta
  pages : Array[Page]
}

pub fn page_count(doc : Document) -> Int {
  doc.pages.length()
}
