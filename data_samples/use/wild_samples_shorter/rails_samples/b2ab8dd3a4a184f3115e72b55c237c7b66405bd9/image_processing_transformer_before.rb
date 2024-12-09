  module Transformers
    class ImageProcessingTransformer < Transformer
      private
        def process(file, format:)
          processor.
            source(file).
            loader(page: 0).

        def operations
          transformations.each_with_object([]) do |(name, argument), list|
            if name.to_s == "combine_options"
              raise ArgumentError, <<~ERROR.squish
                Active Storage's ImageProcessing transformer doesn't support :combine_options,
                as it always generates a single command.
            end
          end
        end
    end
  end
end